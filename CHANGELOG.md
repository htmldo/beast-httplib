# Changelog

All notable changes to this project will be documented in this file.

## [0.1.0] - 2024-10-26

### Added
- Initial release of httplib based on Boost.Beast
- HTTP Server with route handling (GET, POST, PUT, PATCH, DELETE, OPTIONS)
- HTTP Client with full method support
- WebSocket Server with event-driven API
- WebSocket Client with event-driven API
- Request/Response structures compatible with cpp-httplib
- Query parameter parsing
- Path parameter matching with regex
- Headers management
- Static file serving
- Error handling and logging
- Keep-alive support
- Timeout configuration
- Header-only library design
- Examples for HTTP and WebSocket usage
- Basic test suite
- CMake and Makefile build support
- MIT License

### Features
- cpp-httplib compatible API
- C++11 standard compliance
- Thread-per-connection model
- Regex-based routing
- Event callbacks for WebSocket
- Text and binary WebSocket messages
- Automatic WebSocket handshake
- Connection management
- Remote endpoint information

### Documentation
- Comprehensive README with usage examples
- API documentation
- Building instructions
- License information
