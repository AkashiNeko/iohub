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

class Select : public PollerBase {
    std::queue<fd_event_t> event_queue_;
    std::vector<unsigned char> fd_hasharr_;
    size_t max_, size_, readsz_, writesz_, exceptsz_;
    fd_set readfds_, writefds_, exceptfds_;
    bool is_open_;

public:
    Select();
    virtual ~Select() override = default;

    virtual void insert(int fd, int events) override;
    virtual void erase(int fd) override;
    virtual void modify(int fd, int events) override;
    virtual int get_event(int fd) const noexcept override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual fd_event_t wait(int timeout = -1);

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Select

} // namespace iohub

#endif // IOHUB_SELECT_H
