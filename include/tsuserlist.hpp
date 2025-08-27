#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "tsqueue.hpp"
#include "message.hpp"

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

    void printList() {
        std::cout << "Database online:\n";
        forEach([](auto& user) {
            std::cout << user.first << '\n';
        });
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

    template<typename F>
    void forEach(F&& func) const {
        std::scoped_lock lock(mutex_);
        for (const auto& item : database_) {
            func(item);
        }
    }
};