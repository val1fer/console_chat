#pragma once

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <utility>

struct Message {
private:
    size_t _author;
    std::shared_ptr<std::string> _data;
public:

public:
    Message() : _author(0), _data(nullptr) {};
    
    Message(size_t id, std::string&& str) 
        : _author(id), _data(std::make_shared<std::string>(std::move(str))) {};
    
    size_t getID() const { return _author; }
    
    std::string& getData() const { 
        return *_data;
    }
    
    size_t size() const { 
        return _data ? _data->size() : 0; 
    }

    auto getData() {
        return &(*_data.get());
    }

    bool empty() {
        return _data->empty();
    }

    void clear() {
        _author = 0;
        _data->clear();
    }

    friend std::ostream& operator<<(std::ostream& os, const Message& msg) {
        os << msg.getData();
        return os;
    }
};