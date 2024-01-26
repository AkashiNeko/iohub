// Poll.h

#pragma once
#ifndef IOHUB_POLL_H
#define IOHUB_POLL_H

// C++
#include <queue>
#include <map>

// Linux
#include <unistd.h>
#include <sys/select.h>

// iohub
#include "except.h"
#include "PollerBase.h"

namespace iohub {

class Poll : public PollerBase {
    std::vector<unsigned char> fdarr_;
    size_t max_;
    size_t size_;
    std::queue<FD_Event> event_queue_;

public:
    Poll();
    virtual ~Poll() override = default;

    virtual bool insert(int fd, int events) noexcept override;
    virtual bool erase(int fd) noexcept override;
    virtual bool modify(int fd, int events) noexcept override;
    virtual int get_event(int fd) const noexcept override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Poll

} // namespace iohub

#endif // IOHUB_POLL_H
