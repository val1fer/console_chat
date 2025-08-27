#pragma once

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <string>
#include <memory>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"


class Connection;

namespace asio = boost::asio;
using boost::system::error_code;
using message_handler = std::function<void(Message)>;
using error_handler = std::function<void(size_t id)>;
using nickname_handler = std::function<void(std::shared_ptr<Connection>)>;

enum { SERVER_ID = 0 };


class Connection : public std::enable_shared_from_this<Connection> {
private:
    asio::ip::tcp::socket socket_;
    TSQueue<Message> outgoingMessages_;
    TSUserList& userList_;
    size_t id_;
    std::string username_;

    message_handler on_message_;    
    error_handler on_error_; 
    nickname_handler onReadyCallback_;

    asio::streambuf streambuf_; 

    void on_read(error_code error, size_t bytes);
    void on_write(error_code ec, size_t);

public:
    Connection(asio::ip::tcp::socket socket
        , TSUserList& userList
        , size_t id
        , nickname_handler callback);

    void askNickname();
    void readNickname();
    void start(message_handler&& msg_func, error_handler&& err_func);
    void post(const Message& msg);
    void async_read();
    void async_write();
    void disconnect();
    std::string getUsername() const noexcept;
};