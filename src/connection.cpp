#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <sstream>
#include <string>
#include <iostream>
#include <functional>
#include <memory>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"
#include "connection.hpp"

namespace asio = boost::asio;
using boost::system::error_code;

using message_handler = std::function<void(Message)>;
using error_handler = std::function<void(size_t id)>;
using nickname_handler = std::function<void(std::shared_ptr<Connection>)>;


void Connection::on_read(error_code error, size_t bytes) {
    if (!error) {
        std::stringstream message;
        message << username_ << ": "
                << std::istream(&streambuf_).rdbuf();
        streambuf_.consume(bytes);
        on_message_(Message(id_, message.str()));
        async_read();
    } else {
        std::cout << "[Connection] Reading error\n";
        disconnect();
    }
}

void Connection::on_write(error_code ec, size_t) {
    if (!ec) {
        outgoingMessages_.pop();
        if (!outgoingMessages_.empty())
            async_write();
    } else {
        std::cout << ec.message();
        disconnect();
    }
}

Connection::Connection(asio::ip::tcp::socket socket
        , TSUserList& userList
        , size_t id
        , nickname_handler callback) 
            : socket_(std::move(socket))
            , userList_(userList)
            , id_(id)
            , onReadyCallback_(callback) {
        std::cout << "[Connection] established\n";
        askNickname();
    }

    void Connection::askNickname() {
    boost::asio::async_write(socket_, boost::asio::buffer(username_),
        [this](error_code ec, std::size_t) {
            if (!ec) {
                asio::post([this]() { readNickname();});
            } else {
                std::cout << ec.message();
                disconnect();
            }
        });
}

void Connection::readNickname() {
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, streambuf_, '\n',
        [this, self](error_code ec, size_t bytes) {
            if (!ec) { 
                std::istream is(&streambuf_);
                std::getline(is, username_);
                streambuf_.consume(bytes); 
                if (username_.empty()) {
                    askNickname();
                } else {
                    std::cout << "[Connection] " << username_ << " has joined\n";
                    if (onReadyCallback_)
                        onReadyCallback_(self);
                }
            } else {
                std::cout << "Failed to connect to chat\n";
                disconnect();
            }
        });
}


void Connection::start(message_handler&& msg_func, error_handler&& err_func) {
    on_message_ = std::move(msg_func);
    on_error_ = std::move(err_func);
    async_read();
}

void Connection::post(const Message& msg) {
    bool isWriting = !outgoingMessages_.empty();

    if (msg.getID() == id_ || msg.empty()) return;
    else if (msg.getID() == SERVER_ID) {
        std::string newStr = "[Server] " + msg.getData();
        outgoingMessages_.push(Message(SERVER_ID, std::move(newStr)));
    } 
    else outgoingMessages_.push(msg);

    if (!isWriting)
        async_write();
}

void Connection::async_read() {
    auto self = shared_from_this();
    asio::async_read_until(
        socket_,
        streambuf_,
        "\n",
        [this, self](error_code ec, size_t bytes) {
            on_read(ec, bytes);
        }
    );
}

void Connection::async_write() {
    auto self = shared_from_this();
    asio::async_write(
        socket_,
        asio::buffer(outgoingMessages_.front().getData()),
        [this, self](error_code ec, size_t bytes) {
            on_write(ec, bytes);
        }
    );
}

void Connection::disconnect() {
    auto self = shared_from_this();
    asio::post([self, this]() {
        boost::system::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
        on_error_(id_);
    });
}

std::string Connection::getUsername() const noexcept
{ return username_; }
