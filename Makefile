# Makefile for httplib examples and tests
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I./include
LDFLAGS = -lpthread -lboost_system

# Detect Boost location
BOOST_LIBS = $(shell pkg-config --libs boost 2>/dev/null || echo "-lboost_system")
LDFLAGS += $(BOOST_LIBS)

.PHONY: all clean examples tests

all: examples tests

examples: http_server http_client ws_server ws_client

tests:
	@echo "To run cpp-httplib test suite, see tests/Makefile"

http_server: examples/http_server.cpp include/httplib.h
	$(CXX) $(CXXFLAGS) examples/http_server.cpp -o http_server $(LDFLAGS)

http_client: examples/http_client.cpp include/httplib.h
	$(CXX) $(CXXFLAGS) examples/http_client.cpp -o http_client $(LDFLAGS)

ws_server: examples/ws_server.cpp include/httplib.h
	$(CXX) $(CXXFLAGS) examples/ws_server.cpp -o ws_server $(LDFLAGS)

ws_client: examples/ws_client.cpp include/httplib.h
	$(CXX) $(CXXFLAGS) examples/ws_client.cpp -o ws_client $(LDFLAGS)

clean:
	rm -f http_server http_client ws_server ws_client

install:
	install -D -m 644 include/httplib.h $(DESTDIR)/usr/local/include/httplib.h

.DEFAULT_GOAL := all
