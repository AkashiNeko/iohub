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
using fd_event_t = std::pair<int, int>;

enum Event {
    IOHUB_IN  = 0x01,
    IOHUB_PRI = 0x02,
    IOHUB_OUT = 0x04,
}; // Event

class PollerBase {
public:
    virtual bool insert(int fd, int events) noexcept = 0;
    virtual bool erase(int fd) noexcept = 0;
    virtual bool modify(int fd, int events) noexcept = 0;
    virtual int get_event(int fd) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual void clear() noexcept = 0;

    virtual fd_event_t wait(int timeout) = 0;

    virtual bool is_open() const noexcept = 0;
    virtual void close() noexcept = 0;

}; // class PollerBase

// Select

class Select : PollerBase {
    std::queue<fd_event_t> event_queue_;
    std::vector<unsigned char> fdarr_;
    size_t max_, size_, read_sz_, write_sz_, except_sz_;
    fd_set readfds_, writefds_, exceptfds_;
    bool is_open_;

public:
    Select();
    virtual ~Select() = default;

    virtual bool insert(int fd, int events) noexcept override;
    virtual bool erase(int fd) noexcept override;
    virtual bool modify(int fd, int events) noexcept override;
    virtual int get_event(int fd) const noexcept override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual fd_event_t wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Select

// Poll
class Poll : PollerBase {
    std::vector<unsigned char> fdarr_;
    size_t max_;
    size_t size_;
    std::queue<fd_event_t> event_queue_;

public:
    Poll();
    virtual ~Poll() = default;

    virtual bool insert(int fd, int events) noexcept override;
    virtual bool erase(int fd) noexcept override;
    virtual bool modify(int fd, int events) noexcept override;
    virtual int get_event(int fd) const noexcept override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual fd_event_t wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Poll

// Epoll
class Epoll : public PollerBase {
    int epoll_fd_;
    std::unordered_map<int, int> fd_map_;
    std::queue<fd_event_t> event_queue_;

public:
    Epoll();
    virtual ~Epoll() = default;

    virtual bool insert(int fd, int events) noexcept override;
    virtual bool erase(int fd) noexcept override;
    virtual bool modify(int fd, int events) noexcept override;
    virtual int get_event(int fd) const noexcept override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual fd_event_t wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Epoll

} // namespace iohub

#endif // IOHUB_H