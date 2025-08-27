#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <optional>
#include <memory>
#include <cstdint>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"
#include "connection.hpp"
#include "functions.hpp"
#include "server.hpp"

namespace asio = boost::asio;

Server::Server(asio::io_context& context, uint16_t port) :
    context_(context),
    acceptor_(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
        std::cout << "[Server] Started\n";
    }

void Server::post(const Message& message) {
    connections_.forEach([&message](auto& client) {
        client->post(message);
    });
}

void Server::async_accept() {
    socket_.emplace(context_);
    acceptor_.async_accept(*socket_, [&](boost::system::error_code error) {
        auto client = std::make_shared<Connection>(std::move(*socket_), userList_, id_
        ,  [this](std::shared_ptr<Connection> readyConn) {
                        onConnectionReady(readyConn);
                    });
        client->post(Message(SERVER_ID, "You've been connected\nPlease, write your nickname\n"));
            connections_.push(client);
        userList_.addUser(id_++);
        async_accept();
    });
}

void Server::onConnectionReady(std::shared_ptr<Connection> conn) {
    Message msg(SERVER_ID, "Welcome to chat, " + conn->getUsername() + "\n");
    conn->post(msg);
    conn->start(
        std::bind(&Server::post, this, std::placeholders::_1), //func to call server->post(msg) w/o server ref
        [this, weak = std::weak_ptr(conn)] (size_t id) { // error handler
            if (auto left_conn = weak.lock()
            ; left_conn && connections_.erase(left_conn)) {
                std::stringstream ss;
                ss << left_conn->getUsername() << " disconnected\n";
                post(Message(SERVER_ID, ss.str()));
            }
        });
    }


int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: ./server.exe or ./server.exe 5454, where 5454 - any valid port (0-65535)\n";
        return 1;
    }
    asio::io_context context;
    std::optional<Server> srv;
    if (argc == 2) {
        try {
            check_port(argv[1]);
        } catch (std::exception& ex) {
            std::ignore = ex;
            std::cerr << "Write a valid port\n";
            return 1;
        }
        std::string port = argv[1];
        srv.emplace(context, std::stoi(port));
    } else {
        srv.emplace(context, DEFAULT_PORT);
    }
    srv->async_accept();
    context.run();
    return 0;
}