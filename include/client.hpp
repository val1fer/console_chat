#pragma once

#include <boost/asio.hpp>
#include <string>

namespace asio = boost::asio;

class Client {
private:
    asio::io_context& context_;
    asio::ip::tcp::socket socket_;

public:
    Client(asio::io_context& context, const std::string_view ip, const std::string_view port);
    ~Client();
    void connect(const asio::ip::tcp::resolver::results_type& endpoints);
    void writeMessages();
    void send(const std::string& message);
    void getMessages();
    void disconnect();
};