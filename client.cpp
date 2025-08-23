#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <memory>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"

namespace asio = boost::asio;
using boost::system::error_code;

class Client {
private:
    asio::io_context& _context;
    asio::ip::tcp::socket _socket;

public:
    Client(asio::io_context& context, std::string_view ip, std::string_view port) 
            : _context(context)
            , _socket(_context) {
        asio::ip::tcp::resolver resolver(_context);
        auto endpoints = resolver.resolve(ip, port);
        connect(endpoints);
    }
    ~Client() { disconnect(); }

    void connect(const asio::ip::tcp::resolver::results_type& endpoints) {
        boost::asio::async_connect(_socket
            , endpoints
            , [this](error_code ec, asio::ip::tcp::endpoint) {
            if (!ec) {
                asio::post(_context, [this](){ getMessages(); });
                asio::post(_context, [this](){ writeMessages(); });
            } else {
                std::cout << ec.message();
                disconnect();
            }
        });
    }

    void writeMessages() {
        std::thread([this]() {
            std::string str;
            while (_socket.is_open()) {
                if (!std::getline(std::cin, str)) break;
                str+= '\n';
                asio::post(_context, [this, msg = std::move(str)]() mutable {
                    send(std::move(msg));
                });
            }
            disconnect();
        }).detach();
    }

    void send(const std::string& message) {    
        auto msg_ptr = std::make_shared<std::string>(message);
        boost::asio::async_write(_socket, asio::buffer(*msg_ptr),
            [this, msg_ptr](error_code ec, size_t) {
                if (ec) {
                    std::cout << "Write failed: " << ec.message() << '\n';
                    disconnect();
                }
            });
    }

    void getMessages() {
        auto buffer = std::make_shared<std::array<char, 1024>>();
        _socket.async_read_some(asio::buffer(*buffer),
            [this, buffer](error_code ec, size_t bytes) {
                if (!ec) {
                    std::cout.write(buffer->data(), bytes);
                    getMessages();
                } else {
                    std::cout << "Read failed: " << ec.message() << '\n';
                    disconnect();
                }
            });
    }

    void disconnect() {
        std::cout << "[Client] disconnect\n";
        boost::asio::post([this]() { _socket.close(); });
    }
};

int main() {
    std::string port = "60000";
    asio::io_context context;
    Client client(context, "127.0.0.1", port);
    context.run();
}