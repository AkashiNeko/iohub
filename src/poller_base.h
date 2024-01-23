// poller_base.h

#pragma once
#ifndef IOHUB_POLLER_BASE_H
#define IOHUB_POLLER_BASE_H

// C++
#include <utility>

namespace iohub {

using EpollEvent = std::pair<int, int>;

class PollerBase {
public:
    virtual ~PollerBase() = 0;
    
    virtual bool insert(int fd, int events) = 0;
    virtual bool erase(int fd) = 0;
    virtual bool modify(int fd, int events) = 0;
    virtual EpollEvent wait(int timeout = -1) = 0;
};

} // namespace iohub

#endif // IOHUB_POLLER_BASE_H
