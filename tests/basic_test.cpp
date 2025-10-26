#include "../include/httplib.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>

void test_http_server_client() {
    std::cout << "Testing HTTP Server/Client..." << std::endl;
    
    auto server = std::make_shared<httplib::Server>();
    
    server->Get("/test", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Test OK", "text/plain");
    });
    
    server->Get(R"(/num/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        auto num = req.matches[1].str();
        res.set_content("Number: " + num, "text/plain");
    });
    
    server->Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(req.body, "text/plain");
    });
    
    std::thread server_thread([server]() {
        server->listen("127.0.0.1", 18080);
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    httplib::Client cli("127.0.0.1", 18080);
    
    if (auto res = cli.Get("/test")) {
        std::cout << "✓ GET /test: " << res->status << " " << res->body << std::endl;
    } else {
        std::cout << "✗ GET /test failed" << std::endl;
    }
    
    if (auto res = cli.Get("/num/42")) {
        std::cout << "✓ GET /num/42: " << res->status << " " << res->body << std::endl;
    } else {
        std::cout << "✗ GET /num/42 failed" << std::endl;
    }
    
    if (auto res = cli.Post("/echo", "Hello World", "text/plain")) {
        std::cout << "✓ POST /echo: " << res->status << " " << res->body << std::endl;
    } else {
        std::cout << "✗ POST /echo failed" << std::endl;
    }
    
    server->stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    std::cout << "HTTP Server/Client tests completed!" << std::endl << std::endl;
}

void test_websocket_server_client() {
    std::cout << "Testing WebSocket Server/Client..." << std::endl;
    
    std::atomic<bool> server_received{false};
    std::atomic<bool> client_received{false};
    
    auto ws_server = std::make_shared<httplib::WebSocketServer>();
    
    ws_server->on_message([&](httplib::WebSocketConnection& conn, const httplib::WebSocketMessage& msg) {
        std::cout << "Server received: " << msg.data << std::endl;
        server_received = true;
        conn.send("Echo: " + msg.data);
    });
    
    ws_server->on_error([&](httplib::WebSocketConnection&, const std::string& error) {
        std::cerr << "WebSocket server error: " << error << std::endl;
    });
    
    std::thread server_thread([ws_server]() {
        ws_server->listen("127.0.0.1", 19090);
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::thread client_thread([&]() {
        httplib::WebSocketClient ws_cli("127.0.0.1", 19090, "/");
        
        ws_cli.on_message([&](const httplib::WebSocketMessage& msg) {
            std::cout << "Client received: " << msg.data << std::endl;
            client_received = true;
        });
        
        ws_cli.on_error([&](const std::string& error) {
            std::cerr << "WebSocket client error: " << error << std::endl;
        });
        
        if (ws_cli.connect()) {
            ws_cli.send("Test Message");
            
            for (int i = 0; i < 50 && !client_received; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            ws_cli.close();
        }
    });
    
    if (client_thread.joinable()) {
        client_thread.join();
    }
    
    ws_server->stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    if (server_received && client_received) {
        std::cout << "✓ WebSocket echo test passed" << std::endl;
    } else {
        std::cout << "✗ WebSocket echo test failed" << std::endl;
    }
    
    std::cout << "WebSocket Server/Client tests completed!" << std::endl << std::endl;
}

int main() {
    std::cout << "=== httplib Boost.Beast Tests ===" << std::endl << std::endl;
    
    try {
        test_http_server_client();
        test_websocket_server_client();
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}
