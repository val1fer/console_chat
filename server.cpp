#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <optional>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"
#include "connection.hpp"

namespace asio = boost::asio;

class Server {
private:
    asio::io_context& _context;
    asio::ip::tcp::acceptor _acceptor;
    std::optional<asio::ip::tcp::socket> _socket;
    TSQueue<std::shared_ptr<Connection>> _connections;

    TSUserList _userList;
    std::atomic<size_t> _id = 10000;
    enum {SERVER_ID = 0};

public:
    Server(asio::io_context& context, std::uint16_t port) :
        _context(context),
        _acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
            std::cout << "[Server] Started\n";
        }

    void post(const Message& message) {
        for (auto& client : _connections) {
            client->post(message);
        }
    }

    void async_accept() {
        _socket.emplace(_context);
        _acceptor.async_accept(*_socket, [&](boost::system::error_code error) {
            auto client = std::make_shared<Connection>(std::move(*_socket), _userList, _id
        ,  [this](std::shared_ptr<Connection> readyConn) {
                            onConnectionReady(readyConn);
                        });
            client->post(Message(SERVER_ID, "You've been connected\nPlease, write your nickname\n"));
             _connections.push(client);
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
                ; left_conn && _connections.erase(left_conn)) {
                    std::stringstream ss;
                    ss << left_conn->getUsername() << " disconnected\n";
                    post(Message(SERVER_ID, ss.str()));
                }
            });
        }
};

int main(int argc, char* argv[]) {
    asio::io_context io_context;
    Server srv(io_context, 60000);
    srv.async_accept();
    io_context.run();
    return 0;
}