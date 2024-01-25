// Select.h

#pragma once
#ifndef IOHUB_SELECT_H
#define IOHUB_SELECT_H

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

class Select : PollerBase {
    std::map<int, int> fd_map_;
    std::queue<FD_Event> event_queue_;

public:
    Select();
    virtual ~Select() = default;

    virtual bool insert(int fd, int events) override;
    virtual bool erase(int fd) override;
    virtual bool modify(int fd, int events) override;
    virtual int get_event(int fd) const override;
    virtual size_t size() const override;

    virtual FD_Event wait(int timeout = -1);

    virtual bool is_open() const override;
    virtual void close() override;

}; // class Select

} // namespace iohub

#endif // IOHUB_SELECT_H
