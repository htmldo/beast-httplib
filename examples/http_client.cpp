#include "../include/httplib.h"
#include <iostream>
#include <sstream>

int main() {
    // Create HTTP client connecting to localhost:8080
    httplib::Client cli("localhost", 8080);

    // Perform GET request
    if (auto res = cli.Get("/hi")) {
        std::cout << "GET /hi => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // GET with query parameters
    if (auto res = cli.Get("/query?name=Alice&city=Wonderland")) {
        std::cout << "GET /query => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // GET with regex matching
    if (auto res = cli.Get("/numbers/42")) {
        std::cout << "GET /numbers/42 => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // POST request with plain text body
    if (auto res = cli.Post("/echo", "Hello from client", "text/plain")) {
        std::cout << "POST /echo => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // POST request with form data
    httplib::Params params;
    params.emplace("username", "testuser");
    params.emplace("password", "secret");
    if (auto res = cli.Post("/login", params)) {
        std::cout << "POST /login => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // PUT request
    if (auto res = cli.Put("/update", "Updated content", "text/plain")) {
        std::cout << "PUT /update => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // DELETE request
    if (auto res = cli.Delete("/item/42")) {
        std::cout << "DELETE /item/42 => " << res->status << "\n";
        std::cout << res->body << "\n\n";
    }

    // OPTIONS request
    if (auto res = cli.Options("/options")) {
        std::cout << "OPTIONS /options => " << res->status << "\n";
        for (const auto& header : res->headers) {
            std::cout << header.first << ": " << header.second << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "HTTP client operations complete." << std::endl;
    return 0;
}
