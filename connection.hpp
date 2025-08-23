#pragma once

#include <boost/asio.hpp>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"

class Connection;

namespace asio = boost::asio;
using boost::system::error_code;
using message_handler = std::function<void(Message)>;
using error_handler = std::function<void(size_t id)>;
using nickname_handler = std::function<void(std::shared_ptr<Connection>)>;


class Connection : public std::enable_shared_from_this<Connection> {
private:
    asio::ip::tcp::socket _socket;
    TSQueue<Message> _outgoingMessages;
    TSUserList& _userList;
    size_t _id;
    std::string _username;
    enum {SERVER_ID = 0};

    message_handler on_message;    
    error_handler on_error; 
    nickname_handler _onReadyCallback;

    asio::streambuf streambuf; 

public:
    Connection(asio::ip::tcp::socket socket
        , TSUserList& userList
        , size_t id
        , std::function<void(std::shared_ptr<Connection>)> callback) 
                : _socket(std::move(socket))
                , _userList(userList)
                , _id(id)
                , _onReadyCallback(callback) {
            std::cout << "[Connection] established\n";
            askNickname();
        }

        void askNickname() {
        boost::asio::async_write(_socket, boost::asio::buffer(_username),
            [this](error_code ec, std::size_t) {
                if (!ec) {
                    asio::post([this]() { readNickname();});
                } else {
                    std::cout << ec.message();
                    disconnect();
                }
            });
    }

    void readNickname() {
        auto self = shared_from_this();
        boost::asio::async_read_until(_socket, streambuf, '\n',
            [this, self](error_code ec, size_t bytes) {
                if (!ec) { 
                    std::istream is(&streambuf);
                    std::getline(is, _username);
                    streambuf.consume(bytes); 
                    if (_username.empty()) {
                        askNickname();
                    } else {
                        std::cout << "[Connection] " << _username << " has joined\n";
                        if (_onReadyCallback)
                            _onReadyCallback(self);
                    }
                } else {
                    std::cout << "Failed to connect to chat\n";
                    _userList.removeUser(_id);
                    _socket.close();
                }
            });
    }


    void start(message_handler&& msg_func, error_handler&& err_func) {
        on_message = std::move(msg_func);
        on_error = std::move(err_func);
        async_read();
    }

    void post(const Message& msg) {
        bool isWriting = !_outgoingMessages.empty();

        if (msg.getID() == _id || msg.empty()) return;
        else if (msg.getID() == SERVER_ID) {
            std::string newStr = "[Server] " + msg.getData();
            _outgoingMessages.push(Message(SERVER_ID, std::move(newStr)));
        } 
        else _outgoingMessages.push(msg);

        if (!isWriting)
            async_write();
    }

    void async_read() {
        auto self = shared_from_this();
        asio::async_read_until(
            _socket,
            streambuf,
            "\n",
            [this, self](error_code ec, size_t bytes) {
                on_read(ec, bytes);
            }
        );
    }

    void async_write() {
        auto self = shared_from_this();
        asio::async_write(
            _socket,
            asio::buffer(_outgoingMessages.front().getData()),
            [this, self](error_code ec, size_t bytes) {
                on_write(ec, bytes);
            }
        );
    }

    void on_read(error_code error, size_t bytes) {
        if (!error) {
            std::stringstream message;
            message << _username << ": "
                    << std::istream(&streambuf).rdbuf();
            streambuf.consume(bytes);
            on_message(Message(_id, message.str()));
            async_read();
        } else {
            std::cout << "[Connection] Reading error\n";
            disconnect();
        }
    }

    void on_write(error_code ec, size_t) {
        if (!ec) {
            _outgoingMessages.pop();
            if (!_outgoingMessages.empty())
                async_write();
        } else {
            std::cout << ec.message();
            disconnect();
        }
    }

    void disconnect() {
        _userList.removeUser(_id);
        _socket.close();
        on_error(_id);
    }

    auto getUsername() const noexcept { return _username;}
};