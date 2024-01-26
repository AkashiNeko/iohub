// PollerBase.h

#pragma once
#ifndef IOHUB_POLLER_BASE_H
#define IOHUB_POLLER_BASE_H

// C++
#include <utility>

namespace iohub {

// pair {fd: int, event: int}
using fd_event_t = std::pair<int, int>;

enum Event {
    IOHUB_IN  = 0x01,
    IOHUB_PRI = 0x02,
    IOHUB_OUT = 0x04,
}; // Event

class PollerBase {
public:
    // ctor & dtor
    PollerBase() = default;
    virtual ~PollerBase() = 0;

    // uncopyable
    PollerBase(const PollerBase&) = delete;
    PollerBase& operator=(const PollerBase&) = delete;

    virtual bool insert(int fd, int events) noexcept = 0;
    virtual bool erase(int fd) noexcept = 0;
    virtual bool modify(int fd, int events) noexcept = 0;
    virtual int get_event(int fd) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual void clear() noexcept = 0;

    virtual fd_event_t wait(int timeout = -1) = 0;

    virtual bool is_open() const noexcept = 0;
    virtual void close() noexcept = 0;

}; // class PollerBase

} // namespace iohub

#endif // IOHUB_POLLER_BASE_H
