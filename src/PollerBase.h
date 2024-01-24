// PollerBase.h

#pragma once
#ifndef IOHUB_POLLER_BASE_H
#define IOHUB_POLLER_BASE_H

// C++
#include <utility>

namespace iohub {

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

    virtual FD_Event wait(int timeout) = 0;

    virtual bool is_open() = 0;
    virtual void close() = 0;

}; // class PollerBase

} // namespace iohub

#endif // IOHUB_POLLER_BASE_H