#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "tsqueue.hpp"
#include "message.hpp"

namespace asio = boost::asio;

class TSUserList : public std::enable_shared_from_this<TSUserList> {
private:
    std::unordered_map<size_t, std::shared_ptr<TSQueue<Message>>> _database;
    mutable std::mutex _mutex;
public:
    TSUserList() = default;
    TSUserList(TSUserList&) = delete;

   void addUser(size_t id) {
        std::scoped_lock lock(_mutex);
        _database[id] = std::make_shared<TSQueue<Message>>();
        std::cout << "[Users] " << id << " was inserted\n";
    }

    auto& getDatabase() {
        return _database;
    }

    void printList() {
        std::cout << "Database online:\n";
        for (auto& user: _database) {
            std::cout << user.first << ' ';
        }
    }
    
    bool removeUser(size_t id) {
        std::scoped_lock lock(_mutex);
        auto it = _database.find(id);
        if (it != _database.end()) it->second.reset();
        else {
            std::cout << "Couldn't find " << id << " to erase\n";
            printList();
            return false;
        }
        std::cout << "[Users] " << id << " was erased\n";
        return _database.erase(id) > 0;
    }
    
    std::shared_ptr<TSQueue<Message>> getUserQueue(size_t id) const {
        std::scoped_lock lock(_mutex);
        auto it = _database.find(id);
        return (it != _database.end()) ? it->second : nullptr;
    }
    
    bool contains(size_t id) const {
        std::scoped_lock lock(_mutex);
        return _database.find(id) != _database.end();
    }

    size_t size() const {
        std::scoped_lock lock(_mutex);
        return _database.size();
    }

    auto begin() const {return _database.begin(); }
    auto end() const {return _database.end(); }
};