#include "../include/httplib.h"
#include <iostream>
#include <sstream>

int main() {
    httplib::Server svr;

    // Simple GET endpoint
    svr.Get("/hi", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    // GET with path parameter (regex)
    svr.Get(R"(/numbers/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        auto number = req.matches[1].str();
        res.set_content("Number: " + number, "text/plain");
    });

    // GET with query parameters
    svr.Get("/query", [](const httplib::Request& req, httplib::Response& res) {
        std::ostringstream oss;
        oss << "Query parameters:\n";
        for (const auto& param : req.params) {
            oss << param.first << " = " << param.second << "\n";
        }
        res.set_content(oss.str(), "text/plain");
    });

    // POST endpoint
    svr.Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(req.body, "text/plain");
    });

    // JSON endpoint
    svr.Post("/json", [](const httplib::Request& req, httplib::Response& res) {
        std::string json_response = R"({"status": "ok", "received": ")" + req.body + R"("})";
        res.set_content(json_response, "application/json");
    });

    // PUT endpoint
    svr.Put("/update", [](const httplib::Request& req, httplib::Response& res) {
        res.status = 200;
        res.set_content("Updated: " + req.body, "text/plain");
    });

    // DELETE endpoint
    svr.Delete(R"(/item/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        auto id = req.matches[1].str();
        res.set_content("Deleted item: " + id, "text/plain");
    });

    // Error handler
    svr.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        std::string error_msg = "<html><body><h1>404 Not Found</h1></body></html>";
        res.set_content(error_msg, "text/html");
    });

    // Logger
    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        std::cout << req.method << " " << req.path << " -> " << res.status << std::endl;
    });

    std::cout << "HTTP Server listening on http://localhost:8080" << std::endl;
    std::cout << "Try: curl http://localhost:8080/hi" << std::endl;
    std::cout << "Try: curl http://localhost:8080/numbers/123" << std::endl;
    std::cout << "Try: curl http://localhost:8080/query?name=John&age=30" << std::endl;
    std::cout << "Try: curl -X POST -d 'Hello' http://localhost:8080/echo" << std::endl;
    
    svr.listen("0.0.0.0", 8080);

    return 0;
}
