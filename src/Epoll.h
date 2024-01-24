// Epoll.h

#pragma once
#ifndef EPOLL_H
#define EPOLL_H

// C++
#include <queue>

// Linux
#include <unistd.h>
#include <sys/epoll.h>

// iohub
#include "except.h"
#include "PollerBase.h"

namespace iohub {

class Epoll : public PollerBase {
    int epoll_fd_;
    std::queue<EpollEvent> event_queue_;

public:
    Epoll();
    virtual ~Epoll() = default;

    virtual bool insert(int fd, int events) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;

    virtual EpollEvent wait(int timeout = -1);
    virtual void close() override;

}; // class Epoll

} // namespace iohub

#endif // EPOLL_H
