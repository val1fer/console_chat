#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include "functions.hpp"
#include "client.hpp"

enum { DEFAULT_PORT = 60'000 };

using boost::system::error_code;
namespace asio = boost::asio;

Client::Client(asio::io_context& context, const std::string_view ip, const std::string_view port) 
        : context_(context)
        , socket_(context_) {
    asio::ip::tcp::resolver resolver(context_);
    auto endpoints = resolver.resolve(ip, port);
    connect(endpoints);
}

Client::~Client() 
{ disconnect(); }

void Client::connect(const asio::ip::tcp::resolver::results_type& endpoints) {
    asio::async_connect(socket_
        , endpoints
        , [this](error_code ec, asio::ip::tcp::endpoint) {
        if (!ec) {
            asio::post(context_, [this](){ getMessages(); });
            asio::post(context_, [this](){ writeMessages(); });
        } else {
            std::cout << ec.message();
            disconnect();
        }
    });
}

void Client::writeMessages() {
    std::thread([this]() {
        std::string str;
        while (socket_.is_open()) {
            if (!std::getline(std::cin, str)) break;
            str+= '\n';
            asio::post(context_, [this, msg = std::move(str)]() mutable {
                send(std::move(msg));
            });
        }
        disconnect();
    }).detach();
}

void Client::send(const std::string& message) {    
    auto msg_ptr = std::make_shared<std::string>(message);
    boost::asio::async_write(socket_, asio::buffer(*msg_ptr),
        [this, msg_ptr](error_code ec, size_t) {
            if (ec) {
                std::cout << "Write failed: " << ec.message() << '\n';
                disconnect();
            }
        });
}

void Client::getMessages() {
    auto buffer = std::make_shared<std::array<char, 1024>>();
    socket_.async_read_some(asio::buffer(*buffer),
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

void Client::disconnect() {
    std::cout << "[Client] disconnect\n";
    asio::post([this]() {
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket_.close();
    });
}

int main(int argc, char* argv[]) {
    std::string port;
    if (argc > 2) {
        std::cerr << "Usage: ./client.exe or ./client.exe 5454, where 5454 - any valid port (0-65535)\n";
        return 1;
    } else if (argc == 2) {
        try {
            check_port(argv[1]);
        } catch (std::exception& ex) {
            std::ignore = ex;
            std::cerr << "Write a valid port (0-65535)\n";
            return 1;
        }
        port = argv[1];
    } else port = std::to_string(DEFAULT_PORT);
    asio::io_context context;
    Client client(context, "127.0.0.1", port);
    context.run();
    return 0;
}