#pragma once

#include <boost/asio.hpp>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"

namespace asio = boost::asio;
using boost::system::error_code;
using message_handler = std::function<void(Message)>;
using error_handler = std::function<void(size_t id)>;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    asio::ip::tcp::socket _socket;
    TSQueue<Message> _outgoingMessages;
    TSUserList& _userList;
    size_t _id;
    enum {SERVER_ID = 0};

    message_handler on_message;    
    error_handler on_error; 

    asio::streambuf streambuf; 

public:
    Connection(asio::ip::tcp::socket socket, TSUserList& userList, size_t id) 
                : _socket(std::move(socket))
                , _userList(userList)
                , _id(id) {
            std::cout << "[Connection] established\n";
        }

    void start(message_handler&& on_message, error_handler&& on_error) {
        this->on_message = std::move(on_message);
        this->on_error = std::move(on_error);
        async_read();
    }

    void post(const Message& msg) {
        bool isWriting = !_outgoingMessages.empty();
        if (msg.getID() == _id) return;
        else if (msg.getID() == SERVER_ID) {
            std::string newStr = "[Server] " + msg.getData();
            _outgoingMessages.push(Message(SERVER_ID, std::move(newStr)));
        } else _outgoingMessages.push(msg);
        if (!isWriting) {
            async_write();
        }
    }

    void async_read() {
        asio::async_read_until(
            _socket,
            streambuf,
            "\n",
            std::bind(&Connection::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void async_write() {
        asio::async_write(
            _socket,
            asio::buffer(_outgoingMessages.front().getData()),
            std::bind(&Connection::on_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void on_read(error_code error, size_t bytes) {
        if (!error) {
            std::stringstream message;
            message << _id << ": "
                    << std::istream(&streambuf).rdbuf();
            streambuf.consume(bytes);
            on_message(Message(_id, message.str()));
            async_read();
        } else {
            std::cout << "[Connection] Reading error\n";
            on_error(_id);
        }
    }

    void on_write(error_code ec, size_t) {
        if (!ec) {
            _outgoingMessages.pop();
            if (!_outgoingMessages.empty()) {
                async_write();
            }
        } else {
            std::cout << ec.message();
            on_error(_id);
        }
    }

    void disconnect() {
        _userList.removeUser(_id);
        _socket.close();
        on_error(_id);
    }
};