#include "../include/httplib.h"
#include <iostream>
#include <string>

int main() {
    httplib::WebSocketServer ws_svr;

    ws_svr.on_open([](httplib::WebSocketConnection& conn) {
        std::cout << "WebSocket connection opened" << std::endl;
        conn.send("Welcome to the WebSocket echo server!\n");
    });

    ws_svr.on_message([](httplib::WebSocketConnection& conn, const httplib::WebSocketMessage& msg) {
        if (msg.is_binary) {
            std::cout << "Received binary message of size " << msg.data.size() << std::endl;
            conn.send(msg.data.data(), msg.data.size(), true);
        } else {
            std::cout << "Received: " << msg.data << std::endl;
            conn.send("Echo: " + msg.data);
        }
    });

    ws_svr.on_close([](httplib::WebSocketConnection& conn) {
        std::cout << "WebSocket connection closed" << std::endl;
    });

    ws_svr.on_error([](httplib::WebSocketConnection& conn, const std::string& error) {
        std::cerr << "WebSocket error: " << error << std::endl;
    });

    std::cout << "WebSocket Server listening on ws://localhost:9090" << std::endl;
    std::cout << "Connect with: wscat -c ws://localhost:9090" << std::endl;

    ws_svr.listen("0.0.0.0", 9090);

    return 0;
}
