# httplib API Reference

## Table of Contents
- [HTTP Server](#http-server)
- [HTTP Client](#http-client)
- [WebSocket Server](#websocket-server)
- [WebSocket Client](#websocket-client)
- [Data Types](#data-types)

## HTTP Server

### Class: `httplib::Server`

#### Constructor
```cpp
Server();
```

#### Methods

##### Routing Methods
```cpp
Server& Get(const std::string& pattern, Handler handler);
Server& Post(const std::string& pattern, Handler handler);
Server& Put(const std::string& pattern, Handler handler);
Server& Patch(const std::string& pattern, Handler handler);
Server& Delete(const std::string& pattern, Handler handler);
Server& Options(const std::string& pattern, Handler handler);
```

Register a handler for HTTP requests matching the given pattern. The pattern is a regex string.

**Example:**
```cpp
svr.Get("/hello", [](const Request& req, Response& res) {
    res.set_content("Hello!", "text/plain");
});

svr.Get(R"(/users/(\d+))", [](const Request& req, Response& res) {
    auto user_id = req.matches[1].str();
    res.set_content("User ID: " + user_id, "text/plain");
});
```

##### Server Control
```cpp
bool listen(const std::string& host, int port, int socket_flags = 0);
void stop();
bool is_running() const;
```

- `listen()`: Start the server on the specified host and port. This call blocks until the server stops.
- `stop()`: Stop the server.
- `is_running()`: Check if the server is running.

##### Configuration Methods
```cpp
Server& set_logger(Logger logger);
Server& set_error_handler(Handler handler);
Server& set_exception_handler(Handler handler);
Server& set_pre_routing_handler(Handler handler);
Server& set_post_routing_handler(Handler handler);
Server& set_mount_point(const std::string& mount_point, const std::string& dir);
Server& set_base_dir(const std::string& dir);
Server& set_keep_alive_max_count(size_t count);
Server& set_keep_alive_timeout(time_t sec);
Server& set_read_timeout(time_t sec, time_t usec = 0);
Server& set_write_timeout(time_t sec, time_t usec = 0);
Server& set_idle_interval(time_t sec, time_t usec = 0);
Server& set_payload_max_length(size_t length);
```

## HTTP Client

### Class: `httplib::Client`

#### Constructor
```cpp
explicit Client(const std::string& host);
explicit Client(const std::string& host, int port);
```

#### Methods

##### Request Methods
```cpp
std::shared_ptr<Response> Get(const std::string& path);
std::shared_ptr<Response> Get(const std::string& path, const Headers& headers);
std::shared_ptr<Response> Head(const std::string& path);
std::shared_ptr<Response> Head(const std::string& path, const Headers& headers);
std::shared_ptr<Response> Post(const std::string& path, const std::string& body, 
                               const std::string& content_type);
std::shared_ptr<Response> Post(const std::string& path, const Headers& headers,
                               const std::string& body, const std::string& content_type);
std::shared_ptr<Response> Post(const std::string& path, const Params& params);
std::shared_ptr<Response> Put(const std::string& path, const std::string& body,
                              const std::string& content_type);
std::shared_ptr<Response> Patch(const std::string& path, const std::string& body,
                                const std::string& content_type);
std::shared_ptr<Response> Delete(const std::string& path);
std::shared_ptr<Response> Delete(const std::string& path, const Headers& headers,
                                 const std::string& body, const std::string& content_type);
std::shared_ptr<Response> Options(const std::string& path);
std::shared_ptr<Response> Options(const std::string& path, const Headers& headers);
```

##### Configuration Methods
```cpp
void set_connection_timeout(time_t sec, time_t usec = 0);
void set_read_timeout(time_t sec, time_t usec = 0);
void set_write_timeout(time_t sec, time_t usec = 0);
void set_basic_auth(const std::string& username, const std::string& password);
void set_bearer_token_auth(const std::string& token);
void set_keep_alive(bool on);
void set_follow_location(bool on);
void set_compress(bool on);
void set_decompress(bool on);
void set_interface(const std::string& intf);
void set_proxy(const std::string& host, int port);
void set_proxy_basic_auth(const std::string& username, const std::string& password);
void set_proxy_bearer_token_auth(const std::string& token);
void set_logger(Logger logger);
```

## WebSocket Server

### Class: `httplib::WebSocketServer`

#### Constructor
```cpp
WebSocketServer();
```

#### Methods

##### Event Handlers
```cpp
WebSocketServer& on_open(OpenHandler handler);
WebSocketServer& on_message(MessageHandler handler);
WebSocketServer& on_close(CloseHandler handler);
WebSocketServer& on_error(ErrorHandler handler);
```

- `on_open`: Called when a new WebSocket connection is established.
- `on_message`: Called when a message is received.
- `on_close`: Called when a connection is closed.
- `on_error`: Called when an error occurs.

**Handler Types:**
```cpp
using MessageHandler = std::function<void(WebSocketConnection& conn, const WebSocketMessage& msg)>;
using OpenHandler = std::function<void(WebSocketConnection& conn)>;
using CloseHandler = std::function<void(WebSocketConnection& conn)>;
using ErrorHandler = std::function<void(WebSocketConnection& conn, const std::string& error)>;
```

##### Server Control
```cpp
bool listen(const std::string& host, int port);
void stop();
bool is_running() const;
```

**Example:**
```cpp
WebSocketServer ws_svr;

ws_svr.on_open([](WebSocketConnection& conn) {
    std::cout << "Connection opened" << std::endl;
});

ws_svr.on_message([](WebSocketConnection& conn, const WebSocketMessage& msg) {
    conn.send("Echo: " + msg.data);
});

ws_svr.listen("0.0.0.0", 9090);
```

### Class: `httplib::WebSocketConnection`

#### Methods
```cpp
virtual void send(const std::string& message, bool binary = false);
virtual void send(const char* data, size_t length, bool binary = false);
virtual void close();
virtual bool is_open() const;
```

## WebSocket Client

### Class: `httplib::WebSocketClient`

#### Constructor
```cpp
explicit WebSocketClient(const std::string& host, int port, const std::string& path = "/");
```

#### Methods

##### Event Handlers
```cpp
WebSocketClient& on_open(OpenHandler handler);
WebSocketClient& on_message(MessageHandler handler);
WebSocketClient& on_close(CloseHandler handler);
WebSocketClient& on_error(ErrorHandler handler);
```

**Handler Types:**
```cpp
using MessageHandler = std::function<void(const WebSocketMessage& msg)>;
using OpenHandler = std::function<void()>;
using CloseHandler = std::function<void()>;
using ErrorHandler = std::function<void(const std::string& error)>;
```

##### Connection Control
```cpp
bool connect();
void close();
bool is_connected() const;
```

##### Send Methods
```cpp
void send(const std::string& message, bool binary = false);
void send(const char* data, size_t length, bool binary = false);
```

**Example:**
```cpp
WebSocketClient ws_cli("localhost", 9090, "/");

ws_cli.on_open([]() {
    std::cout << "Connected" << std::endl;
});

ws_cli.on_message([](const WebSocketMessage& msg) {
    std::cout << "Received: " << msg.data << std::endl;
});

if (ws_cli.connect()) {
    ws_cli.send("Hello!");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ws_cli.close();
}
```

## Data Types

### struct Request
```cpp
struct Request {
    std::string method;
    std::string path;
    std::string version;
    Headers headers;
    std::string body;
    Params params;          // Query parameters
    Match matches;          // Regex matches
    MultipartFormDataItems files;
    std::string remote_addr;
    int remote_port;
    std::string local_addr;
    int local_port;
    
    bool has_header(const std::string& key) const;
    std::string get_header_value(const std::string& key, size_t id = 0) const;
    size_t get_header_value_count(const std::string& key) const;
    void set_header(const std::string& key, const std::string& val);
    bool has_param(const std::string& key) const;
    std::string get_param_value(const std::string& key, size_t id = 0) const;
    size_t get_param_value_count(const std::string& key) const;
    bool has_file(const std::string& key) const;
    MultipartFormData get_file_value(const std::string& key) const;
};
```

### struct Response
```cpp
struct Response {
    int status = 200;
    std::string version = "HTTP/1.1";
    Headers headers;
    std::string body;
    std::string location;
    
    bool has_header(const std::string& key) const;
    std::string get_header_value(const std::string& key, size_t id = 0) const;
    size_t get_header_value_count(const std::string& key) const;
    void set_header(const std::string& key, const std::string& val);
    void set_content(const std::string& s, const std::string& content_type);
    void set_redirect(const std::string& url, int status_code = 302);
};
```

### class WebSocketMessage
```cpp
class WebSocketMessage {
public:
    std::string data;
    bool is_binary = false;
    
    WebSocketMessage();
    WebSocketMessage(const std::string& d, bool binary = false);
};
```

### Type Aliases
```cpp
using Headers = std::multimap<std::string, std::string>;
using Params = std::multimap<std::string, std::string>;
using Match = std::smatch;
using Handler = std::function<void(const Request&, Response&)>;
using Logger = std::function<void(const Request&, const Response&)>;
```

## Error Handling

All methods that can fail return appropriate values:
- HTTP Client methods return `std::shared_ptr<Response>` which can be `nullptr` on failure
- `listen()` returns `bool` indicating success/failure
- `connect()` returns `bool` indicating success/failure

Exceptions are caught internally and logged to `std::cerr`.

## Thread Safety

- Each HTTP connection is handled in a separate thread
- Each WebSocket connection is handled in a separate thread
- The WebSocket client runs a receive loop in a separate thread
- Handler callbacks should be thread-safe if accessing shared data
