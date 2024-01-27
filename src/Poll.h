// Poll.h

#pragma once
#ifndef IOHUB_POLL_H
#define IOHUB_POLL_H

// C++
#include <queue>
#include <unordered_map>
#include <memory>
#include <functional>

// Linux
#include <unistd.h>
#include <sys/poll.h>

// iohub
#include "except.h"
#include "PollerBase.h"

namespace iohub {

class Poll : public PollerBase {
    std::queue<fd_event_t> event_queue_;
    std::unordered_map<int, int> fd_map_;
    std::vector<pollfd> pollfd_arr_;
    bool is_open_;

public:
    Poll();
    virtual ~Poll() override = default;

    virtual void insert(int fd, int events) override;
    virtual void erase(int fd) override;
    virtual void modify(int fd, int events) override;
    virtual int get_event(int fd) const noexcept override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual fd_event_t wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Poll

} // namespace iohub

#endif // IOHUB_POLL_H
