#pragma once

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <mutex>
#include <deque>

template <typename T>
class TSQueue : public std::enable_shared_from_this<TSQueue<T>> {
protected:
    std::mutex mutex_;
    std::deque<T> _deq;
public:
    TSQueue() = default;

    TSQueue(const TSQueue<T>&) = delete;
    TSQueue operator= (const TSQueue<T>& other) = delete;

    TSQueue(TSQueue<T>&& other) {
        std::scoped_lock lock(mutex_);
        swap(other);
    }

    TSQueue operator= (TSQueue<T>&& other) {
        std::scoped_lock lock(mutex_);
        swap(other);
    };

    ~TSQueue() { clear(); }

    const T& front() {
        std::scoped_lock lock(mutex_);
        return _deq.front();
    }

    const T& back() {
        std::scoped_lock lock(mutex_);
        return _deq.back();
    }

    void push(const T& item) {
        std::cout << "push\n";
        std::scoped_lock lock(mutex_);
        _deq.emplace_back(std::move(item));
    }

    void pop() {
        std::cout << "pop\n";
        std::scoped_lock lock(mutex_);
        _deq.pop_front();
    }

    bool erase(const T& item) {
        std::scoped_lock lock(mutex_);
        auto iter = std::find(_deq.begin(), _deq.end(), item);
        if (iter == _deq.end()) return false;
        _deq.erase(iter);
        return true;
    }

    bool empty() {
        std::scoped_lock lock(mutex_);
        return _deq.empty();
    }

    size_t size() {
        std::scoped_lock lock(mutex_);
        return _deq.size();
    }

    void clear() {
        std::scoped_lock lock(mutex_);
        return _deq.clear();
    }

    void print() {
        std::scoped_lock lock(mutex_);
        while (!_deq.empty()) {
            std::cout << "[From Queue] \n"<< _deq.front() << '\n';
            _deq.pop_front();
        }
    }

    void swap(const TSQueue<T>& other) {
        std::scoped_lock lock(mutex_);
        std::swap(_deq, other._deq);
    }

    auto begin() { return _deq.begin(); }
    auto end() { return _deq.end(); }

};