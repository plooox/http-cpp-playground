#ifndef HTTP_CPP_PLAYGROUND_SIMPLETCPSERVER_H
#define HTTP_CPP_PLAYGROUND_SIMPLETCPSERVER_H
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <iostream>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_set>
#include <csignal>

/**
 * Simple TCP Echo Server
 */
class SimpleTCPServer {
public:
    explicit SimpleTCPServer(uint16_t port)
        : m_base(event_base_new(), &event_base_free),
        m_listener(nullptr, &evconnlistener_free),
        m_port(port) {

        if (!m_base) {
            throw std::runtime_error("Failed to create simple tcp echo server");
        }
    }
    ~SimpleTCPServer() {
        for (auto* bev : m_active_connections) {
            bufferevent_free(bev);
        }
    }

    void start() {
        sockaddr_in in{};
        in.sin_family = AF_INET;
        in.sin_addr.s_addr = INADDR_ANY;
        in.sin_port = htons(m_port);

        m_listener.reset(evconnlistener_new_bind(
                    m_base.get(),
                    accept_new_cli_callback,
                    this,
                    LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                    -1,
                    reinterpret_cast<sockaddr*>(&in),
                    sizeof(in)));

        if (!m_listener) {
            throw std::runtime_error("Failed to create listener");
        }

        // Set error callback
        evconnlistener_set_error_cb(m_listener.get(), accept_error_callback);

        // Show server info
        std::cout << "Simple TCP ECHO server is started on port " << m_port << std::endl;

        // Start event loop
        event_base_dispatch(m_base.get());
    }

    void stop() {
        std::cout << "Server is closed\n";
        event_base_loopexit(m_base.get(), nullptr);
    }
private:
    std::unique_ptr<event_base, decltype(&event_base_free)> m_base;
    std::unique_ptr<evconnlistener, decltype(&evconnlistener_free)> m_listener;
    std::unordered_set<bufferevent*> m_active_connections;
    uint16_t m_port;

    // New client connection callback
    static void accept_new_cli_callback(evconnlistener* evconnlistener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx) {
        auto* server = static_cast<SimpleTCPServer*>(ctx);
        server->handle_new_connection(fd, address, socklen);
    }

    // Listener error callback
    static void accept_error_callback(evconnlistener* evconnlistener, void* ctx) {
        auto* server = static_cast<SimpleTCPServer*>(ctx);
        int err = EVUTIL_SOCKET_ERROR();
        std::cerr << "[ERROR] Listener error[" << err << "]: " << evutil_socket_error_to_string(err) << std::endl;
        server->stop();
    }

    // Read callback
    static void read_callback(bufferevent* bev, void* ctx) {
        auto* server = static_cast<SimpleTCPServer*>(ctx);
        server->handle_read(bev);
    }

    // Connection event callback (connection close, error ... )
    static void event_callback(bufferevent* bev, short events, void* ctx) {
        auto* server = static_cast<SimpleTCPServer*>(ctx);
        server->handle_event(bev, events);
    }

    void handle_new_connection(evutil_socket_t fd, sockaddr* address, int socklen) {
        // Client Info
        char host[256];
        char service[16];
        getnameinfo(address, socklen, host, 256, service, 16, NI_NUMERICHOST | NI_NUMERICSERV);
        std::cout << "New Connection: " << host << ":" << service << std::endl;

        // Create Bufferevent - 자동 버퍼링을 제공하는 인터페이스
        bufferevent* bufferevent = bufferevent_socket_new(m_base.get(), fd, BEV_OPT_CLOSE_ON_FREE);
        if (!bufferevent) {
            std::cerr << "Fail to create Bufferevent\n";
            close(fd);
            return;
        }

        // Insert connection pool
        m_active_connections.insert(bufferevent);

        // Set callback
        bufferevent_setcb(bufferevent,
                          read_callback,    // Read callback
                          nullptr,          // Write callback
                          event_callback,  // Connection callback
                          this);            // Callback param


        // Activate READ/WRITE event
        bufferevent_enable(bufferevent, EV_READ | EV_WRITE);

    }

    void handle_read(bufferevent* bev) {
        // Read data from Input buffer
        evbuffer* input = bufferevent_get_input(bev);
        auto len = evbuffer_get_length(input);

        if (len == 0) return;

        auto* line = evbuffer_readln(input, nullptr, EVBUFFER_EOL_ANY);
        if (line) {
            std::string message(line);
            free(line);

            std::cout << "Read message: " << message << std::endl;

            // "Quit" message
            if (message == "quit" || message == "Quit" || message == "q") {
                auto* goodbye_msg = "Good bye!\n";
                bufferevent_write(bev, goodbye_msg, strlen(goodbye_msg));
                bufferevent_free(bev);
                m_active_connections.erase(bev);
                return;
            }

            // Return ECHO message
            std::string echo_msg = "[ECHO] " + message + "\n";
            bufferevent_write(bev, echo_msg.c_str(), echo_msg.length());
        }
    }

    void handle_event(bufferevent* bev, short events) {
        if (events & BEV_EVENT_EOF) {
            std::cout << "Connection is closed\n";
        } else if (events & BEV_EVENT_ERROR) {
            std::cout << "Connection error\n";
        }

        // Remove connection
        m_active_connections.erase(bev);
        bufferevent_free(bev);
    }
};

#endif //HTTP_CPP_PLAYGROUND_SIMPLETCPSERVER_H
