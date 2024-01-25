// Epoll.h

#pragma once
#ifndef IOHUB_EPOLL_H
#define IOHUB_EPOLL_H

// C++
#include <queue>
#include <unordered_map>

// Linux
#include <unistd.h>
#include <sys/epoll.h>

// iohub
#include "except.h"
#include "PollerBase.h"

namespace iohub {

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

#endif // IOHUB_EPOLL_H
