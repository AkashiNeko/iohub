// Select.h

#pragma once
#ifndef IOHUB_SELECT_H
#define IOHUB_SELECT_H

// C++
#include <queue>
#include <set>

// Linux
#include <unistd.h>
#include <sys/select.h>

// iohub
#include "except.h"
#include "PollerBase.h"

namespace iohub {

class Select : PollerBase {
    std::queue<FD_Event> event_queue_;
    std::vector<unsigned char> fdarr_;
    size_t max_index_;
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

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Select

} // namespace iohub

#endif // IOHUB_SELECT_H
