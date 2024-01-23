// epoll.h

#pragma once
#ifndef EPOLL_H
#define EPOLL_H

// C++
#include <queue>
#include <utility>

// Linux
#include <unistd.h>
#include <sys/epoll.h>

// iohub
#include "except.h"
#include "poller_base.h"

namespace iohub {

class Epoll : public PollerBase {
    int epoll_fd_;
    std::queue<EpollEvent> event_queue_;

public:
    Epoll();
    virtual ~Epoll() = default;

    void close();
    virtual bool insert(int fd, int events = EPOLLIN) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;

    virtual EpollEvent wait(int timeout = -1);

}; // class Epoll

} // namespace iohub

#endif // EPOLL_H