#include "../include/httplib.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 9090;
    std::string path = "/";

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }

    httplib::WebSocketClient ws_cli(host, port, path);

    std::atomic<bool> running{true};

    // Handle connection open
    ws_cli.on_open([&]() {
        std::cout << "WebSocket connected to ws://" << host << ":" << port << path << std::endl;
        std::cout << "Type messages and press Enter to send (type 'quit' to exit)" << std::endl;
    });

    // Handle incoming messages
    ws_cli.on_message([](const httplib::WebSocketMessage& msg) {
        if (msg.is_binary) {
            std::cout << "Received binary message: " << msg.data.size() << " bytes" << std::endl;
        } else {
            std::cout << "Received: " << msg.data << std::endl;
        }
    });

    // Handle connection close
    ws_cli.on_close([&]() {
        std::cout << "WebSocket connection closed" << std::endl;
        running = false;
    });

    // Handle errors
    ws_cli.on_error([&](const std::string& error) {
        std::cerr << "WebSocket error: " << error << std::endl;
        running = false;
    });

    // Connect to the WebSocket server
    if (!ws_cli.connect()) {
        std::cerr << "Failed to connect to WebSocket server" << std::endl;
        return 1;
    }

    // Wait a moment for connection to establish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Send some test messages
    ws_cli.send("Hello from WebSocket client!");
    ws_cli.send("This is a test message");

    // Interactive mode: read from stdin and send messages
    std::thread input_thread([&]() {
        std::string line;
        while (running && std::getline(std::cin, line)) {
            if (line == "quit" || line == "exit") {
                running = false;
                break;
            }
            if (!line.empty()) {
                ws_cli.send(line);
            }
        }
        running = false;
    });

    // Keep the connection alive
    while (running && ws_cli.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up
    std::cout << "Closing connection..." << std::endl;
    ws_cli.close();
    
    if (input_thread.joinable()) {
        input_thread.join();
    }

    std::cout << "WebSocket client shut down" << std::endl;
    return 0;
}
