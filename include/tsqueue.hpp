#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <deque>

template <typename T>
class TSQueue : public std::enable_shared_from_this<TSQueue<T>> {
protected:
    mutable std::mutex mutex_;
    std::deque<T> deq_;
public:
    TSQueue() = default;
    ~TSQueue() { clear(); }

    TSQueue(const TSQueue<T>&) = delete;
    TSQueue operator= (const TSQueue<T>& other) = delete;

    TSQueue(TSQueue<T>&& other) {
        std::scoped_lock lock(mutex_);
        swap(other);
    }

    TSQueue& operator= (TSQueue<T>&& other) {
        std::scoped_lock lock(mutex_);
        swap(other);
        return *this;
    };

    const T front() const {
        std::scoped_lock lock(mutex_);
        return deq_.front();
    }

    const T back() const {
        std::scoped_lock lock(mutex_);
        return deq_.back();
    }

    void push(T item) {
        std::scoped_lock lock(mutex_);
        deq_.push_back(std::move(item));
    }

    void pop() {
        std::scoped_lock lock(mutex_);
        deq_.pop_front();
    }

    bool erase(const T& item) {
        std::scoped_lock lock(mutex_);
        auto iter = std::find(deq_.begin(), deq_.end(), item);
        if (iter == deq_.end()) return false;
        deq_.erase(iter);
        return true;
    }

    [[nodiscard]] bool empty() {
        std::scoped_lock lock(mutex_);
        return deq_.empty();
    }

    size_t size() {
        std::scoped_lock lock(mutex_);
        return deq_.size();
    }

    void clear() {
        std::scoped_lock lock(mutex_);
        deq_.clear();
    }

    void print() const {
        std::scoped_lock lock(mutex_);
        for (const auto& item : deq_) {
            std::cout << item << '\n';
        }
    }

    void swap(TSQueue& other) noexcept {
        std::scoped_lock lock(mutex_, other.mutex_);
        std::swap(deq_, other.deq_);
    }

    template<typename F>
    void forEach(F&& func) const {
        std::scoped_lock lock(mutex_);
        for (const auto& item : deq_) {
            func(item);
        }
    }
};