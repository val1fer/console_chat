#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "tsqueue.hpp"
#include "message.hpp"

namespace asio = boost::asio;

class TSUserList : public std::enable_shared_from_this<TSUserList> {
private:
    std::unordered_map<size_t, std::shared_ptr<TSQueue<Message>>> database_;
    mutable std::mutex mutex_;
public:
    TSUserList() = default;
    TSUserList(TSUserList&) = delete;

   void addUser(size_t id) {
        std::scoped_lock lock(mutex_);
        database_[id] = std::make_shared<TSQueue<Message>>();
        std::cout << "[Users] " << id << " was inserted\n";
    }

    auto& getDatabase() {
        return database_;
    }

    void printList() {
        std::cout << "Database online:\n";
        for (auto& user: database_) {
            std::cout << user.first << ' ';
        }
    }
    
    bool removeUser(size_t id) {
        std::scoped_lock lock(mutex_);
        auto it = database_.find(id);
        if (it != database_.end()) it->second.reset();
        else {
            std::cout << "Couldn't find " << id << " to erase\n";
            printList();
            return false;
        }
        std::cout << "[Users] " << id << " was erased\n";
        return database_.erase(id) > 0;
    }
    
    std::shared_ptr<TSQueue<Message>> getUserQueue(size_t id) const {
        std::scoped_lock lock(mutex_);
        auto it = database_.find(id);
        return (it != database_.end()) ? it->second : nullptr;
    }
    
    bool contains(size_t id) const {
        std::scoped_lock lock(mutex_);
        return database_.find(id) != database_.end();
    }

    size_t size() const {
        std::scoped_lock lock(mutex_);
        return database_.size();
    }

    auto begin() const {return database_.begin(); }
    auto end() const {return database_.end(); }
};