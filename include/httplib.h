#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H

/*
 * httplib.h - A C++ HTTP/HTTPS/WebSocket library based on Boost.Beast
 * Compatible with cpp-httplib API
 */

#define CPPHTTPLIB_VERSION "0.1.0"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/config.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <iterator>
#include <regex>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <fstream>
#include <atomic>
#include <ctime>

namespace httplib {

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

using Headers = std::multimap<std::string, std::string>;
using Params = std::multimap<std::string, std::string>;
using Match = std::smatch;

struct MultipartFormData {
    std::string name;
    std::string content;
    std::string filename;
    std::string content_type;
};

using MultipartFormDataItems = std::vector<MultipartFormData>;

struct Request {
    std::string method;
    std::string path;
    std::string version;
    Headers headers;
    std::string body;
    
    // Query parameters
    Params params;
    
    // Path parameters
    Match matches;
    
    // Multipart form data
    MultipartFormDataItems files;
    
    // Remote address
    std::string remote_addr;
    int remote_port = 0;
    
    // Local address
    std::string local_addr;
    int local_port = 0;
    
    // Helper functions
    bool has_header(const std::string &key) const {
        return headers.find(key) != headers.end();
    }
    
    std::string get_header_value(const std::string &key, size_t id = 0) const {
        auto it = headers.find(key);
        if (it == headers.end()) return "";
        std::advance(it, id);
        if (it != headers.end() && it->first == key) {
            return it->second;
        }
        return "";
    }
    
    size_t get_header_value_count(const std::string &key) const {
        return headers.count(key);
    }
    
    void set_header(const std::string &key, const std::string &val) {
        headers.emplace(key, val);
    }
    
    bool has_param(const std::string &key) const {
        return params.find(key) != params.end();
    }
    
    std::string get_param_value(const std::string &key, size_t id = 0) const {
        auto it = params.find(key);
        if (it == params.end()) return "";
        std::advance(it, id);
        if (it != params.end() && it->first == key) {
            return it->second;
        }
        return "";
    }
    
    size_t get_param_value_count(const std::string &key) const {
        return params.count(key);
    }
    
    bool has_file(const std::string &key) const {
        for (const auto &file : files) {
            if (file.name == key) return true;
        }
        return false;
    }
    
    MultipartFormData get_file_value(const std::string &key) const {
        for (const auto &file : files) {
            if (file.name == key) return file;
        }
        return MultipartFormData();
    }
};

struct Response {
    int status = 200;
    std::string version = "HTTP/1.1";
    Headers headers;
    std::string body;
    std::string location;
    
    // Content provider
    std::function<bool(size_t offset, size_t length, std::string &data)> content_provider;
    size_t content_length = 0;
    
    bool has_header(const std::string &key) const {
        return headers.find(key) != headers.end();
    }
    
    std::string get_header_value(const std::string &key, size_t id = 0) const {
        auto it = headers.find(key);
        if (it == headers.end()) return "";
        std::advance(it, id);
        if (it != headers.end() && it->first == key) {
            return it->second;
        }
        return "";
    }
    
    size_t get_header_value_count(const std::string &key) const {
        return headers.count(key);
    }
    
    void set_header(const std::string &key, const std::string &val) {
        headers.emplace(key, val);
    }
    
    void set_content(const std::string &s, const std::string &content_type) {
        body = s;
        set_header("Content-Type", content_type);
    }
    
    void set_content_provider(
        size_t length,
        const std::string &content_type,
        std::function<bool(size_t offset, size_t length, std::string &data)> provider) {
        content_length = length;
        content_provider = provider;
        set_header("Content-Type", content_type);
    }
    
    void set_redirect(const std::string &url, int status_code = 302) {
        status = status_code;
        set_header("Location", url);
    }
};

class Stream {
public:
    virtual ~Stream() = default;
    virtual bool is_readable() const = 0;
    virtual bool is_writable() const = 0;
    virtual ssize_t read(char *ptr, size_t size) = 0;
    virtual ssize_t write(const char *ptr, size_t size) = 0;
    virtual void get_remote_ip_and_port(std::string &ip, int &port) const = 0;
};

using Handler = std::function<void(const Request &, Response &)>;
using Logger = std::function<void(const Request &, const Response &)>;

namespace detail {

inline std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

inline void parse_query_text(const std::string &s, Params &params) {
    std::string key;
    std::string val;
    
    size_t i = 0;
    while (i < s.size()) {
        key.clear();
        val.clear();
        
        // Parse key
        while (i < s.size() && s[i] != '=' && s[i] != '&') {
            if (s[i] == '%' && i + 2 < s.size()) {
                auto hex = s.substr(i + 1, 2);
                key += static_cast<char>(std::stoi(hex, nullptr, 16));
                i += 3;
            } else if (s[i] == '+') {
                key += ' ';
                i++;
            } else {
                key += s[i];
                i++;
            }
        }
        
        if (i < s.size() && s[i] == '=') {
            i++; // skip '='
            
            // Parse value
            while (i < s.size() && s[i] != '&') {
                if (s[i] == '%' && i + 2 < s.size()) {
                    auto hex = s.substr(i + 1, 2);
                    val += static_cast<char>(std::stoi(hex, nullptr, 16));
                    i += 3;
                } else if (s[i] == '+') {
                    val += ' ';
                    i++;
                } else {
                    val += s[i];
                    i++;
                }
            }
        }
        
        if (!key.empty()) {
            params.emplace(key, val);
        }
        
        if (i < s.size() && s[i] == '&') {
            i++;
        }
    }
}

} // namespace detail

class Server {
public:
    using Handler = httplib::Handler;
    using Logger = httplib::Logger;
    
    Server() 
        : is_running_(false)
        , new_task_queue_max_size_(0)
        , payload_max_length_(1024 * 1024 * 512) // 512MB
    {
    }
    
    virtual ~Server() {
        stop();
    }
    
    Server &Get(const std::string &pattern, Handler handler) {
        get_handlers_.emplace_back(std::make_pair(std::regex(pattern), handler));
        return *this;
    }
    
    Server &Post(const std::string &pattern, Handler handler) {
        post_handlers_.emplace_back(std::make_pair(std::regex(pattern), handler));
        return *this;
    }
    
    Server &Put(const std::string &pattern, Handler handler) {
        put_handlers_.emplace_back(std::make_pair(std::regex(pattern), handler));
        return *this;
    }
    
    Server &Patch(const std::string &pattern, Handler handler) {
        patch_handlers_.emplace_back(std::make_pair(std::regex(pattern), handler));
        return *this;
    }
    
    Server &Delete(const std::string &pattern, Handler handler) {
        delete_handlers_.emplace_back(std::make_pair(std::regex(pattern), handler));
        return *this;
    }
    
    Server &Options(const std::string &pattern, Handler handler) {
        options_handlers_.emplace_back(std::make_pair(std::regex(pattern), handler));
        return *this;
    }
    
    bool listen(const std::string &host, int port, int socket_flags = 0) {
        return listen_internal(host, port, socket_flags);
    }
    
    void stop() {
        is_running_ = false;
        if (acceptor_) {
            beast::error_code ec;
            acceptor_->close(ec);
        }
        if (ioc_) {
            ioc_->stop();
        }
    }
    
    bool is_running() const {
        return is_running_;
    }
    
    Server &set_logger(Logger logger) {
        logger_ = logger;
        return *this;
    }
    
    Server &set_error_handler(Handler handler) {
        error_handler_ = handler;
        return *this;
    }
    
    Server &set_exception_handler(Handler handler) {
        exception_handler_ = handler;
        return *this;
    }
    
    Server &set_pre_routing_handler(Handler handler) {
        pre_routing_handler_ = handler;
        return *this;
    }
    
    Server &set_post_routing_handler(Handler handler) {
        post_routing_handler_ = handler;
        return *this;
    }
    
    Server &set_mount_point(const std::string &mount_point, const std::string &dir) {
        mount_points_.emplace(mount_point, dir);
        return *this;
    }
    
    Server &set_base_dir(const std::string &dir) {
        base_dir_ = dir;
        return *this;
    }
    
    Server &set_keep_alive_max_count(size_t count) {
        keep_alive_max_count_ = count;
        return *this;
    }
    
    Server &set_keep_alive_timeout(time_t sec) {
        keep_alive_timeout_sec_ = sec;
        return *this;
    }
    
    Server &set_read_timeout(time_t sec, time_t usec = 0) {
        read_timeout_sec_ = sec;
        read_timeout_usec_ = usec;
        return *this;
    }
    
    Server &set_write_timeout(time_t sec, time_t usec = 0) {
        write_timeout_sec_ = sec;
        write_timeout_usec_ = usec;
        return *this;
    }
    
    Server &set_idle_interval(time_t sec, time_t usec = 0) {
        idle_interval_sec_ = sec;
        idle_interval_usec_ = usec;
        return *this;
    }
    
    Server &set_payload_max_length(size_t length) {
        payload_max_length_ = length;
        return *this;
    }
    
protected:
    bool listen_internal(const std::string &host, int port, int socket_flags) {
        (void)socket_flags;
        try {
            is_running_ = true;
            
            ioc_ = std::make_shared<net::io_context>();
            
            auto const address = net::ip::make_address(host);
            auto const port_num = static_cast<unsigned short>(port);
            
            acceptor_ = std::make_shared<tcp::acceptor>(*ioc_, tcp::endpoint{address, port_num});
            
            while (is_running_) {
                try {
                    tcp::socket socket{*ioc_};
                    acceptor_->accept(socket);
                    
                    std::thread([this, s = std::move(socket)]() mutable {
                        handle_session(std::move(s));
                    }).detach();
                    
                } catch (std::exception const &e) {
                    if (is_running_) {
                        std::cerr << "Accept error: " << e.what() << std::endl;
                    }
                    break;
                }
            }
            
            is_running_ = false;
            acceptor_.reset();
            ioc_.reset();
            
            return true;
            
        } catch (std::exception const &e) {
            std::cerr << "Listen error: " << e.what() << std::endl;
            is_running_ = false;
            return false;
        }
    }
    
    void handle_session(tcp::socket socket) {
        try {
            beast::flat_buffer buffer;
            
            for (;;) {
                http::request<http::string_body> req;
                http::read(socket, buffer, req);
                
                Request request;
                Response response;
                
                // Convert beast request to our Request
                request.method = std::string(req.method_string());
                request.path = std::string(req.target());
                request.version = "HTTP/" + std::to_string(req.version() / 10) + "." + 
                                  std::to_string(req.version() % 10);
                
                // Parse path and query
                std::string path_str(req.target());
                auto query_pos = path_str.find('?');
                if (query_pos != std::string::npos) {
                    request.path = path_str.substr(0, query_pos);
                    auto query_str = path_str.substr(query_pos + 1);
                    detail::parse_query_text(query_str, request.params);
                } else {
                    request.path = path_str;
                }
                
                // Copy headers
                for (auto const &field : req) {
                    request.headers.emplace(
                        std::string(field.name_string()),
                        std::string(field.value())
                    );
                }
                
                // Copy body
                request.body = req.body();
                
                // Get remote endpoint info
                try {
                    auto remote_ep = socket.remote_endpoint();
                    request.remote_addr = remote_ep.address().to_string();
                    request.remote_port = remote_ep.port();
                } catch (...) {
                    // Ignore endpoint errors
                }
                
                // Route the request
                route_request(request, response);
                
                // Convert our Response to beast response
                http::response<http::string_body> res;
                res.version(req.version());
                res.result(static_cast<http::status>(response.status));
                res.keep_alive(req.keep_alive());
                
                // Copy headers
                for (auto const &header : response.headers) {
                    res.set(header.first, header.second);
                }
                
                // Set body
                res.body() = response.body;
                res.prepare_payload();
                
                // Send response
                http::write(socket, res);
                
                // Check if we should keep alive
                if (!req.keep_alive()) {
                    break;
                }
            }
            
            socket.shutdown(tcp::socket::shutdown_send);
            
        } catch (beast::system_error const &se) {
            if (se.code() != http::error::end_of_stream) {
                std::cerr << "Session error: " << se.what() << std::endl;
            }
        } catch (std::exception const &e) {
            std::cerr << "Session error: " << e.what() << std::endl;
        }
    }
    
    void route_request(Request &req, Response &res) {
        bool handled = false;
        
        // Pre-routing handler
        if (pre_routing_handler_) {
            pre_routing_handler_(req, res);
        }
        
        // Find matching handler
        auto &handlers = get_handlers_for_method(req.method);
        for (auto &handler_pair : handlers) {
            std::smatch matches;
            if (std::regex_match(req.path, matches, handler_pair.first)) {
                req.matches = matches;
                try {
                    handler_pair.second(req, res);
                    handled = true;
                } catch (std::exception const &e) {
                    if (exception_handler_) {
                        exception_handler_(req, res);
                    } else {
                        res.status = 500;
                        res.set_content("Internal Server Error", "text/plain");
                    }
                    handled = true;
                }
                break;
            }
        }
        
        // Check mount points for file serving
        if (!handled && !base_dir_.empty()) {
            handled = serve_static_file(req, res);
        }
        
        // Error handler
        if (!handled) {
            if (error_handler_) {
                error_handler_(req, res);
            } else {
                res.status = 404;
                res.set_content("Not Found", "text/plain");
            }
        }
        
        // Post-routing handler
        if (post_routing_handler_) {
            post_routing_handler_(req, res);
        }
        
        // Logger
        if (logger_) {
            logger_(req, res);
        }
    }
    
    std::vector<std::pair<std::regex, Handler>> &get_handlers_for_method(const std::string &method) {
        if (method == "GET") return get_handlers_;
        if (method == "POST") return post_handlers_;
        if (method == "PUT") return put_handlers_;
        if (method == "PATCH") return patch_handlers_;
        if (method == "DELETE") return delete_handlers_;
        if (method == "OPTIONS") return options_handlers_;
        return get_handlers_; // default
    }
    
    bool serve_static_file(const Request &req, Response &res) {
        std::string file_path = base_dir_ + req.path;
        
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        std::ostringstream ss;
        ss << file.rdbuf();
        
        res.status = 200;
        res.body = ss.str();
        
        // Set content type based on extension
        auto ext_pos = file_path.rfind('.');
        if (ext_pos != std::string::npos) {
            auto ext = file_path.substr(ext_pos);
            if (ext == ".html" || ext == ".htm") {
                res.set_header("Content-Type", "text/html");
            } else if (ext == ".css") {
                res.set_header("Content-Type", "text/css");
            } else if (ext == ".js") {
                res.set_header("Content-Type", "application/javascript");
            } else if (ext == ".json") {
                res.set_header("Content-Type", "application/json");
            } else if (ext == ".png") {
                res.set_header("Content-Type", "image/png");
            } else if (ext == ".jpg" || ext == ".jpeg") {
                res.set_header("Content-Type", "image/jpeg");
            } else if (ext == ".gif") {
                res.set_header("Content-Type", "image/gif");
            } else if (ext == ".svg") {
                res.set_header("Content-Type", "image/svg+xml");
            } else {
                res.set_header("Content-Type", "application/octet-stream");
            }
        }
        
        return true;
    }
    
    std::atomic<bool> is_running_;
    std::shared_ptr<net::io_context> ioc_;
    std::shared_ptr<tcp::acceptor> acceptor_;
    
    std::vector<std::pair<std::regex, Handler>> get_handlers_;
    std::vector<std::pair<std::regex, Handler>> post_handlers_;
    std::vector<std::pair<std::regex, Handler>> put_handlers_;
    std::vector<std::pair<std::regex, Handler>> patch_handlers_;
    std::vector<std::pair<std::regex, Handler>> delete_handlers_;
    std::vector<std::pair<std::regex, Handler>> options_handlers_;
    
    Handler error_handler_;
    Handler exception_handler_;
    Handler pre_routing_handler_;
    Handler post_routing_handler_;
    Logger logger_;
    
    std::map<std::string, std::string> mount_points_;
    std::string base_dir_;
    
    size_t keep_alive_max_count_ = 5;
    time_t keep_alive_timeout_sec_ = 5;
    time_t read_timeout_sec_ = 5;
    time_t read_timeout_usec_ = 0;
    time_t write_timeout_sec_ = 5;
    time_t write_timeout_usec_ = 0;
    time_t idle_interval_sec_ = 0;
    time_t idle_interval_usec_ = 0;
    size_t payload_max_length_;
    size_t new_task_queue_max_size_;
};

class Client {
public:
    explicit Client(const std::string &host)
        : host_(host), port_(80) {
        parse_host();
    }
    
    explicit Client(const std::string &host, int port)
        : host_(host), port_(port) {
    }
    
    virtual ~Client() = default;
    
    std::shared_ptr<Response> Get(const std::string &path) {
        return Get(path, Headers{});
    }
    
    std::shared_ptr<Response> Get(const std::string &path, const Headers &headers) {
        return send_request("GET", path, headers, "");
    }
    
    std::shared_ptr<Response> Head(const std::string &path) {
        return Head(path, Headers{});
    }
    
    std::shared_ptr<Response> Head(const std::string &path, const Headers &headers) {
        return send_request("HEAD", path, headers, "");
    }
    
    std::shared_ptr<Response> Post(const std::string &path, const std::string &body, 
                                    const std::string &content_type) {
        Headers headers;
        headers.emplace("Content-Type", content_type);
        return send_request("POST", path, headers, body);
    }
    
    std::shared_ptr<Response> Post(const std::string &path, const Headers &headers, 
                                     const std::string &body, const std::string &content_type) {
        Headers h = headers;
        h.emplace("Content-Type", content_type);
        return send_request("POST", path, h, body);
    }
    
    std::shared_ptr<Response> Post(const std::string &path, const Params &params) {
        std::ostringstream ss;
        bool first = true;
        for (auto const &param : params) {
            if (!first) ss << "&";
            ss << param.first << "=" << param.second;
            first = false;
        }
        return Post(path, ss.str(), "application/x-www-form-urlencoded");
    }
    
    std::shared_ptr<Response> Put(const std::string &path, const std::string &body, 
                                   const std::string &content_type) {
        Headers headers;
        headers.emplace("Content-Type", content_type);
        return send_request("PUT", path, headers, body);
    }
    
    std::shared_ptr<Response> Patch(const std::string &path, const std::string &body, 
                                     const std::string &content_type) {
        Headers headers;
        headers.emplace("Content-Type", content_type);
        return send_request("PATCH", path, headers, body);
    }
    
    std::shared_ptr<Response> Delete(const std::string &path) {
        return Delete(path, Headers{}, "", "");
    }
    
    std::shared_ptr<Response> Delete(const std::string &path, const Headers &headers, 
                                      const std::string &body, const std::string &content_type) {
        Headers h = headers;
        if (!content_type.empty()) {
            h.emplace("Content-Type", content_type);
        }
        return send_request("DELETE", path, h, body);
    }
    
    std::shared_ptr<Response> Options(const std::string &path) {
        return Options(path, Headers{});
    }
    
    std::shared_ptr<Response> Options(const std::string &path, const Headers &headers) {
        return send_request("OPTIONS", path, headers, "");
    }
    
    void set_connection_timeout(time_t sec, time_t usec = 0) {
        connection_timeout_sec_ = sec;
        connection_timeout_usec_ = usec;
    }
    
    void set_read_timeout(time_t sec, time_t usec = 0) {
        read_timeout_sec_ = sec;
        read_timeout_usec_ = usec;
    }
    
    void set_write_timeout(time_t sec, time_t usec = 0) {
        write_timeout_sec_ = sec;
        write_timeout_usec_ = usec;
    }
    
    void set_basic_auth(const std::string &username, const std::string &password) {
        basic_auth_username_ = username;
        basic_auth_password_ = password;
    }
    
    void set_bearer_token_auth(const std::string &token) {
        bearer_token_ = token;
    }
    
    void set_keep_alive(bool on) {
        keep_alive_ = on;
    }
    
    void set_follow_location(bool on) {
        follow_location_ = on;
    }
    
    void set_compress(bool on) {
        compress_ = on;
    }
    
    void set_decompress(bool on) {
        decompress_ = on;
    }
    
    void set_interface(const std::string &intf) {
        interface_ = intf;
    }
    
    void set_proxy(const std::string &host, int port) {
        proxy_host_ = host;
        proxy_port_ = port;
    }
    
    void set_proxy_basic_auth(const std::string &username, const std::string &password) {
        proxy_basic_auth_username_ = username;
        proxy_basic_auth_password_ = password;
    }
    
    void set_proxy_bearer_token_auth(const std::string &token) {
        proxy_bearer_token_ = token;
    }
    
    void set_logger(Logger logger) {
        logger_ = logger;
    }
    
protected:
    void parse_host() {
        // Parse host:port if present
        auto colon_pos = host_.find(':');
        if (colon_pos != std::string::npos) {
            port_ = std::stoi(host_.substr(colon_pos + 1));
            host_ = host_.substr(0, colon_pos);
        }
    }
    
    std::shared_ptr<Response> send_request(const std::string &method, const std::string &path,
                                            const Headers &headers, const std::string &body) {
        try {
            net::io_context ioc;
            tcp::resolver resolver{ioc};
            tcp::socket socket{ioc};
            
            auto const results = resolver.resolve(host_, std::to_string(port_));
            net::connect(socket, results.begin(), results.end());
            
            http::request<http::string_body> req;
            req.method(http::string_to_verb(method));
            req.target(path);
            req.version(11);
            req.set(http::field::host, host_);
            req.set(http::field::user_agent, "httplib/0.1.0");
            
            // Set headers
            for (auto const &header : headers) {
                req.set(header.first, header.second);
            }
            
            // Set body
            if (!body.empty()) {
                req.body() = body;
                req.prepare_payload();
            }
            
            // Send request
            http::write(socket, req);
            
            // Read response
            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(socket, buffer, res);
            
            // Convert to our Response
            auto response = std::make_shared<Response>();
            response->status = static_cast<int>(res.result());
            response->body = res.body();
            
            // Copy headers
            for (auto const &field : res) {
                response->headers.emplace(
                    std::string(field.name_string()),
                    std::string(field.value())
                );
            }
            
            socket.shutdown(tcp::socket::shutdown_both);
            
            return response;
            
        } catch (std::exception const &e) {
            std::cerr << "Request error: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    std::string host_;
    int port_;
    time_t connection_timeout_sec_ = 300;
    time_t connection_timeout_usec_ = 0;
    time_t read_timeout_sec_ = 300;
    time_t read_timeout_usec_ = 0;
    time_t write_timeout_sec_ = 300;
    time_t write_timeout_usec_ = 0;
    std::string basic_auth_username_;
    std::string basic_auth_password_;
    std::string bearer_token_;
    bool keep_alive_ = false;
    bool follow_location_ = false;
    bool compress_ = false;
    bool decompress_ = true;
    std::string interface_;
    std::string proxy_host_;
    int proxy_port_ = -1;
    std::string proxy_basic_auth_username_;
    std::string proxy_basic_auth_password_;
    std::string proxy_bearer_token_;
    Logger logger_;
};

// WebSocket support

class WebSocketMessage {
public:
    std::string data;
    bool is_binary = false;
    
    WebSocketMessage() = default;
    WebSocketMessage(const std::string &d, bool binary = false) 
        : data(d), is_binary(binary) {}
};

class WebSocketConnection {
public:
    virtual ~WebSocketConnection() = default;
    
    virtual void send(const std::string &message, bool binary = false) = 0;
    virtual void send(const char *data, size_t length, bool binary = false) = 0;
    virtual void close() = 0;
    virtual bool is_open() const = 0;
};

namespace detail {

class WebSocketConnectionImpl : public WebSocketConnection {
public:
    explicit WebSocketConnectionImpl(tcp::socket socket)
        : ws_(std::move(socket)) {
    }
    
    void accept() {
        ws_.accept();
    }
    
    void handshake(const std::string &host, const std::string &path) {
        ws_.handshake(host, path);
    }
    
    void send(const std::string &message, bool binary = false) override {
        send(message.data(), message.size(), binary);
    }
    
    void send(const char *data, size_t length, bool binary = false) override {
        try {
            ws_.binary(binary);
            ws_.write(net::buffer(data, length));
        } catch (std::exception const &e) {
            std::cerr << "WebSocket send error: " << e.what() << std::endl;
        }
    }
    
    void close() override {
        try {
            ws_.close(websocket::close_code::normal);
        } catch (...) {
            // Ignore close errors
        }
    }
    
    bool is_open() const override {
        return ws_.is_open();
    }
    
    bool read(WebSocketMessage &msg) {
        try {
            beast::flat_buffer buffer;
            ws_.read(buffer);
            
            msg.data = beast::buffers_to_string(buffer.data());
            msg.is_binary = ws_.got_binary();
            
            return true;
        } catch (beast::system_error const &se) {
            if (se.code() != websocket::error::closed) {
                std::cerr << "WebSocket read error: " << se.what() << std::endl;
            }
            return false;
        } catch (std::exception const &e) {
            std::cerr << "WebSocket read error: " << e.what() << std::endl;
            return false;
        }
    }
    
private:
    websocket::stream<tcp::socket> ws_;
};

} // namespace detail

class WebSocketServer {
public:
    using MessageHandler = std::function<void(WebSocketConnection &conn, const WebSocketMessage &msg)>;
    using OpenHandler = std::function<void(WebSocketConnection &conn)>;
    using CloseHandler = std::function<void(WebSocketConnection &conn)>;
    using ErrorHandler = std::function<void(WebSocketConnection &conn, const std::string &error)>;
    
    WebSocketServer() : is_running_(false) {}
    
    virtual ~WebSocketServer() {
        stop();
    }
    
    WebSocketServer &on_message(MessageHandler handler) {
        message_handler_ = handler;
        return *this;
    }
    
    WebSocketServer &on_open(OpenHandler handler) {
        open_handler_ = handler;
        return *this;
    }
    
    WebSocketServer &on_close(CloseHandler handler) {
        close_handler_ = handler;
        return *this;
    }
    
    WebSocketServer &on_error(ErrorHandler handler) {
        error_handler_ = handler;
        return *this;
    }
    
    bool listen(const std::string &host, int port) {
        try {
            is_running_ = true;
            
            ioc_ = std::make_shared<net::io_context>();
            
            auto const address = net::ip::make_address(host);
            auto const port_num = static_cast<unsigned short>(port);
            
            acceptor_ = std::make_shared<tcp::acceptor>(*ioc_, tcp::endpoint{address, port_num});
            
            while (is_running_) {
                try {
                    tcp::socket socket{*ioc_};
                    acceptor_->accept(socket);
                    
                    std::thread([this, s = std::move(socket)]() mutable {
                        handle_websocket_session(std::move(s));
                    }).detach();
                    
                } catch (std::exception const &e) {
                    if (is_running_) {
                        std::cerr << "WebSocket accept error: " << e.what() << std::endl;
                    }
                    break;
                }
            }
            
            is_running_ = false;
            acceptor_.reset();
            ioc_.reset();
            
            return true;
            
        } catch (std::exception const &e) {
            std::cerr << "WebSocket listen error: " << e.what() << std::endl;
            is_running_ = false;
            return false;
        }
    }
    
    void stop() {
        is_running_ = false;
        if (acceptor_) {
            beast::error_code ec;
            acceptor_->close(ec);
        }
        if (ioc_) {
            ioc_->stop();
        }
    }
    
    bool is_running() const {
        return is_running_;
    }
    
protected:
    void handle_websocket_session(tcp::socket socket) {
        std::shared_ptr<detail::WebSocketConnectionImpl> ws_conn;
        try {
            ws_conn = std::make_shared<detail::WebSocketConnectionImpl>(std::move(socket));
            ws_conn->accept();
            
            if (open_handler_) {
                open_handler_(*ws_conn);
            }
            
            while (ws_conn->is_open()) {
                WebSocketMessage msg;
                if (ws_conn->read(msg)) {
                    if (message_handler_) {
                        message_handler_(*ws_conn, msg);
                    }
                } else {
                    if (ws_conn->is_open() && error_handler_) {
                        error_handler_(*ws_conn, "WebSocket read error");
                    }
                    break;
                }
            }
            
            if (close_handler_) {
                close_handler_(*ws_conn);
            }
            
        } catch (std::exception const &e) {
            if (error_handler_ && ws_conn) {
                error_handler_(*ws_conn, e.what());
            }
            std::cerr << "WebSocket session error: " << e.what() << std::endl;
        }
    }
    
    std::atomic<bool> is_running_;
    std::shared_ptr<net::io_context> ioc_;
    std::shared_ptr<tcp::acceptor> acceptor_;
    
    MessageHandler message_handler_;
    OpenHandler open_handler_;
    CloseHandler close_handler_;
    ErrorHandler error_handler_;
};

class WebSocketClient {
public:
    using MessageHandler = std::function<void(const WebSocketMessage &msg)>;
    using OpenHandler = std::function<void()>;
    using CloseHandler = std::function<void()>;
    using ErrorHandler = std::function<void(const std::string &error)>;
    
    explicit WebSocketClient(const std::string &host, int port, const std::string &path = "/")
        : host_(host), port_(port), path_(path), is_connected_(false) {
    }
    
    virtual ~WebSocketClient() {
        close();
    }
    
    WebSocketClient &on_message(MessageHandler handler) {
        message_handler_ = handler;
        return *this;
    }
    
    WebSocketClient &on_open(OpenHandler handler) {
        open_handler_ = handler;
        return *this;
    }
    
    WebSocketClient &on_close(CloseHandler handler) {
        close_handler_ = handler;
        return *this;
    }
    
    WebSocketClient &on_error(ErrorHandler handler) {
        error_handler_ = handler;
        return *this;
    }
    
    bool connect() {
        try {
            ioc_ = std::make_shared<net::io_context>();
            
            tcp::resolver resolver{*ioc_};
            auto const results = resolver.resolve(host_, std::to_string(port_));
            
            tcp::socket socket{*ioc_};
            net::connect(socket, results.begin(), results.end());
            
            auto ws_impl = std::make_shared<detail::WebSocketConnectionImpl>(std::move(socket));
            
            // Perform WebSocket handshake
            std::string host_header = host_;
            if (port_ != 80 && port_ != 443) {
                host_header += ":" + std::to_string(port_);
            }
            ws_impl->handshake(host_header, path_);
            
            ws_conn_ = ws_impl;
            is_connected_ = true;
            
            // Call open handler
            if (open_handler_) {
                open_handler_();
            }
            
            // Start receive thread
            receive_thread_ = std::make_unique<std::thread>([this]() {
                receive_loop();
            });
            
            return true;
            
        } catch (std::exception const &e) {
            if (error_handler_) {
                error_handler_(std::string("Connect error: ") + e.what());
            }
            std::cerr << "WebSocket connect error: " << e.what() << std::endl;
            return false;
        }
    }
    
    void send(const std::string &message, bool binary = false) {
        if (ws_conn_ && is_connected_) {
            ws_conn_->send(message, binary);
        }
    }
    
    void send(const char *data, size_t length, bool binary = false) {
        if (ws_conn_ && is_connected_) {
            ws_conn_->send(data, length, binary);
        }
    }
    
    void close() {
        is_connected_ = false;
        
        if (ws_conn_) {
            ws_conn_->close();
        }
        
        if (receive_thread_ && receive_thread_->joinable()) {
            receive_thread_->join();
        }
        
        if (close_handler_) {
            close_handler_();
        }
    }
    
    bool is_connected() const {
        return is_connected_;
    }
    
protected:
    void receive_loop() {
        auto ws_impl = ws_conn_;
        if (!ws_impl) return;
        
        while (is_connected_) {
            WebSocketMessage msg;
            if (ws_impl->read(msg)) {
                if (message_handler_) {
                    message_handler_(msg);
                }
            } else {
                if (ws_impl->is_open() && error_handler_) {
                    error_handler_("WebSocket client read error");
                }
                break;
            }
        }
        
        is_connected_ = false;
    }
    
    std::string host_;
    int port_;
    std::string path_;
    std::atomic<bool> is_connected_;
    
    std::shared_ptr<net::io_context> ioc_;
    std::shared_ptr<detail::WebSocketConnectionImpl> ws_conn_;
    std::unique_ptr<std::thread> receive_thread_;
    
    MessageHandler message_handler_;
    OpenHandler open_handler_;
    CloseHandler close_handler_;
    ErrorHandler error_handler_;
};

} // namespace httplib

#endif // CPPHTTPLIB_HTTPLIB_H
