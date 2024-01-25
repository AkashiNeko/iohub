// nanonet.h

#pragma once
#ifndef IOHUB_H
#define IOHUB_H

// C++
#include <utility>
#include <queue>
#include <string>
#include <exception>
#include <map>
#include <unordered_map>

// Linux
#include <unistd.h>
#include <sys/epoll.h>

namespace iohub {

// except

class IOHubExcept : public std::exception {
    std::string except_msg_;
public:
    explicit IOHubExcept(const std::string& msg) : except_msg_(msg) {}
    explicit IOHubExcept(std::string&& msg) : except_msg_(std::move(msg)) {}
    virtual ~IOHubExcept() override = default;
    virtual const char* what() const noexcept override {
        return except_msg_.c_str();
    }
};

// throw exceptions
#if __cplusplus >= 201703L

template <class ExceptType = IOHubExcept, class ...Args>
inline void assert_throw(bool condition, const Args&... args) {
    if (condition) return;
    std::string s;
    ((s += args), ...);
    throw ExceptType(std::move(s));
}

#else // 201103L <= __cplusplus < 201703L

inline void append_string_(std::string&) {}

template <class T, class ...Args>
inline void append_string_(std::string& s, const T& arg, const Args&... args) {
    s += arg;
    append_string_(s, args...);
}

template <class ExceptType = IOHubExcept, class ...Args>
inline void assert_throw(bool condition, const Args&... args) {
    if (condition) return;
    std::string s;
    append_string_(s, args...);
    throw ExceptType(std::move(s));
}

#endif // __cplusplus

// PollerBase

// pair {fd: int, event: int}
using FD_Event = std::pair<int, int>;

enum Event {
    IOHUB_IN  = 0x01,
    IOHUB_PRI = 0x02,
    IOHUB_OUT = 0x04,
}; // Event

class PollerBase {
public:
    virtual bool insert(int fd, int events) = 0;
    virtual bool erase(int fd) = 0;
    virtual bool modify(int fd, int events) = 0;
    virtual int get_event(int fd) const = 0;
    virtual size_t size() const = 0;

    virtual FD_Event wait(int timeout) = 0;

    virtual bool is_open() const = 0;
    virtual void close() = 0;

}; // class PollerBase

// Select

class Select : PollerBase {
    std::map<int, int> fd_map_;
    std::queue<FD_Event> event_queue_;

public:
    Select();
    virtual ~Select() = default;

    virtual bool insert(int fd, int events) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;
    virtual int get_event(int fd) const override;
    virtual size_t size() const override;

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const override;
    virtual void close() override;

}; // class Select

// Poll
class Poll : PollerBase {
    std::map<int, int> fd_map_;
    std::queue<FD_Event> event_queue_;

public:
    Poll();
    virtual ~Poll() = default;

    virtual bool insert(int fd, int events) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;
    virtual int get_event(int fd) const override;
    virtual size_t size() const override;

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const override;
    virtual void close() override;

}; // class Poll

// Epoll
class Epoll : public PollerBase {
    int epoll_fd_;
    std::unordered_map<int, int> fd_map_;
    std::queue<FD_Event> event_queue_;

public:
    Epoll();
    virtual ~Epoll() = default;

    virtual bool insert(int fd, int events) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;
    virtual int get_event(int fd) const override;
    virtual size_t size() const override;

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const override;
    virtual void close() override;

}; // class Epoll

} // namespace iohub

#endif // IOHUB_H