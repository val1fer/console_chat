#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <optional>
#include <memory>
#include <atomic>
#include <cstdint>

#include "tsqueue.hpp"
#include "tsuserlist.hpp"
#include "message.hpp"
#include "connection.hpp"

namespace asio = boost::asio;
enum { DEFAULT_PORT = 60'000 };

class Server {
private:
    asio::io_context& context_;
    asio::ip::tcp::acceptor acceptor_;
    std::optional<asio::ip::tcp::socket> socket_;
    TSQueue<std::shared_ptr<Connection>> connections_;

    TSUserList userList_;
    std::atomic<size_t> id_ = 10000;

public:
    Server(asio::io_context& context, uint16_t port);
    ~Server() = default;
    void post(const Message& message);
    void async_accept();
    void onConnectionReady(std::shared_ptr<Connection> conn);
};
