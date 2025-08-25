#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <optional>
#include <memory>
#include <atomic>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"
#include "connection.hpp"

namespace asio = boost::asio;

class Server {
private:
    asio::io_context& context_;
    asio::ip::tcp::acceptor acceptor_;
    std::optional<asio::ip::tcp::socket> socket_;
    TSQueue<std::shared_ptr<Connection>> connections_;

    TSUserList _userList;
    std::atomic<size_t> _id = 10000;
    enum {SERVER_ID = 0};

public:
    Server(asio::io_context& context, std::uint16_t port) :
        context_(context),
        acceptor_(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
            std::cout << "[Server] Started\n";
        }

    void post(const Message& message) {
        connections_.forEach([&message](auto& client) {
            client->post(message);
        });
    }

    void async_accept() {
        socket_.emplace(context_);
        acceptor_.async_accept(*socket_, [&](boost::system::error_code error) {
            auto client = std::make_shared<Connection>(std::move(*socket_), _userList, _id
            ,  [this](std::shared_ptr<Connection> readyConn) {
                            onConnectionReady(readyConn);
                        });
            client->post(Message(SERVER_ID, "You've been connected\nPlease, write your nickname\n"));
             connections_.push(client);
            _userList.addUser(_id++);
            async_accept();
        });
    }

    void onConnectionReady(std::shared_ptr<Connection> conn) {
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
};

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: ./server.exe or ./server.exe 5454, where 5454 - any valid port (0-65535)\n";
        return 1;
    }
    asio::io_context context;
    std::optional<Server> srv;
    if (argc == 2) { //custom port
        try {
            int port = std::stoi(argv[1]);
            if (port > UINT16_MAX) std::cerr << "Your port is actually " << port%65535 << "range of valid ports: 0-65535\n";
            srv.emplace(context, port);
        } catch (std::exception& ex) {
            std::ignore = ex;
            std::cerr << "Write a valid port\n";
            return 1;
        }
    } else {
        srv.emplace(context, 60000); //default port
    }
    srv->async_accept();
    context.run();
    return 0;
}