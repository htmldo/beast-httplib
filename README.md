# httplib - HTTP/WebSocket Library based on Boost.Beast

A C++11 header-only HTTP/HTTPS/WebSocket library based on Boost.Beast, compatible with cpp-httplib API.

## Features

- **Header-only**: Just include `httplib.h` and you're ready to go
- **cpp-httplib compatible API**: Drop-in replacement for cpp-httplib
- **Based on Boost.Beast**: Leverages the robust Boost.Beast library
- **HTTP Server & Client**: Full-featured HTTP/1.1 support
- **WebSocket Server & Client**: Real-time bidirectional communication
- **Simple and intuitive API**: Easy to use for both beginners and experts

## Requirements

- C++11 or later
- Boost.Beast (Boost 1.70+)
- Boost.Asio

## Installation

This is a header-only library. Just copy `include/httplib.h` to your project and include it:

```cpp
#include "httplib.h"
```

## HTTP Server Example

```cpp
#include "httplib.h"
#include <iostream>

int main() {
    httplib::Server svr;

    svr.Get("/hi", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.Get(R"(/numbers/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        auto numbers = req.matches[1];
        res.set_content(numbers, "text/plain");
    });

    svr.Post("/post", [](const httplib::Request& req, httplib::Response& res) {
        auto body = req.body;
        res.set_content(body, "text/plain");
    });

    std::cout << "Server listening on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
}
```

## HTTP Client Example

```cpp
#include "httplib.h"
#include <iostream>

int main() {
    httplib::Client cli("localhost", 8080);

    if (auto res = cli.Get("/hi")) {
        std::cout << res->status << std::endl;
        std::cout << res->body << std::endl;
    } else {
        std::cout << "Error: " << res.error() << std::endl;
    }

    // POST with body
    auto res = cli.Post("/post", "test data", "text/plain");
}
```

## WebSocket Server Example

```cpp
#include "httplib.h"
#include <iostream>

int main() {
    httplib::WebSocketServer ws_svr;

    ws_svr.on_open([](httplib::WebSocketConnection& conn) {
        std::cout << "WebSocket connection opened" << std::endl;
    });

    ws_svr.on_message([](httplib::WebSocketConnection& conn, 
                          const httplib::WebSocketMessage& msg) {
        std::cout << "Received: " << msg.data << std::endl;
        
        // Echo the message back
        conn.send("Echo: " + msg.data);
    });

    ws_svr.on_close([](httplib::WebSocketConnection& conn) {
        std::cout << "WebSocket connection closed" << std::endl;
    });

    std::cout << "WebSocket server listening on ws://localhost:9090" << std::endl;
    ws_svr.listen("0.0.0.0", 9090);
}
```

## WebSocket Client Example

```cpp
#include "httplib.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    httplib::WebSocketClient ws_cli("localhost", 9090, "/");

    ws_cli.on_open([]() {
        std::cout << "WebSocket connected" << std::endl;
    });

    ws_cli.on_message([](const httplib::WebSocketMessage& msg) {
        std::cout << "Received: " << msg.data << std::endl;
    });

    ws_cli.on_close([]() {
        std::cout << "WebSocket disconnected" << std::endl;
    });

    if (ws_cli.connect()) {
        // Send messages
        ws_cli.send("Hello WebSocket!");
        ws_cli.send("Another message");
        
        // Keep alive
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        ws_cli.close();
    }
}
```

## API Documentation

### HTTP Server

#### Routing

```cpp
httplib::Server svr;

svr.Get("/path", handler);      // GET request
svr.Post("/path", handler);     // POST request
svr.Put("/path", handler);      // PUT request
svr.Patch("/path", handler);    // PATCH request
svr.Delete("/path", handler);   // DELETE request
svr.Options("/path", handler);  // OPTIONS request
```

#### Request Object

```cpp
struct Request {
    std::string method;
    std::string path;
    std::string version;
    Headers headers;
    std::string body;
    Params params;          // Query parameters
    Match matches;          // Regex matches
    
    bool has_header(const std::string& key);
    std::string get_header_value(const std::string& key);
    bool has_param(const std::string& key);
    std::string get_param_value(const std::string& key);
};
```

#### Response Object

```cpp
struct Response {
    int status = 200;
    Headers headers;
    std::string body;
    
    void set_content(const std::string& s, const std::string& content_type);
    void set_header(const std::string& key, const std::string& val);
    void set_redirect(const std::string& url, int status_code = 302);
};
```

### HTTP Client

```cpp
httplib::Client cli("hostname", port);

auto res = cli.Get("/path");
auto res = cli.Post("/path", body, "text/plain");
auto res = cli.Put("/path", body, "text/plain");
auto res = cli.Patch("/path", body, "text/plain");
auto res = cli.Delete("/path");
```

### WebSocket Server

```cpp
httplib::WebSocketServer ws_svr;

ws_svr.on_open([](WebSocketConnection& conn) { /* ... */ });
ws_svr.on_message([](WebSocketConnection& conn, const WebSocketMessage& msg) { /* ... */ });
ws_svr.on_close([](WebSocketConnection& conn) { /* ... */ });

ws_svr.listen("0.0.0.0", port);
```

### WebSocket Client

```cpp
httplib::WebSocketClient ws_cli("hostname", port, "/path");

ws_cli.on_open([]() { /* ... */ });
ws_cli.on_message([](const WebSocketMessage& msg) { /* ... */ });
ws_cli.on_close([]() { /* ... */ });

ws_cli.connect();
ws_cli.send("message");
ws_cli.close();
```

## Building Examples

### Using Make

```bash
# Build all examples and tests
make

# Build only examples
make examples

# Build only tests
make tests

# Clean build artifacts
make clean
```

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
```

### Manual Compilation

```bash
# Compile HTTP server example
g++ -std=c++11 -I./include examples/http_server.cpp -o http_server -lpthread -lboost_system

# Compile HTTP client example
g++ -std=c++11 -I./include examples/http_client.cpp -o http_client -lpthread -lboost_system

# Compile WebSocket server example
g++ -std=c++11 -I./include examples/ws_server.cpp -o ws_server -lpthread -lboost_system

# Compile WebSocket client example
g++ -std=c++11 -I./include examples/ws_client.cpp -o ws_client -lpthread -lboost_system
```

## cpp-httplib Compatibility

This library is designed to be API-compatible with cpp-httplib. Most code written for cpp-httplib should work with minimal or no modifications.

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
