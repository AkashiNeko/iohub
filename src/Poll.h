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

class Poll : PollerBase {
    std::map<int, int> fd_map_;
    std::queue<FD_Event> event_queue_;

public:
    Poll();
    virtual ~Poll() = default;

    virtual bool insert(int fd, int events) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;
    virtual int get_event(int fd) const override;
    virtual size_t size() const override;

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const override;
    virtual void close() override;

}; // class Poll

} // namespace iohub

#endif // IOHUB_POLL_H
