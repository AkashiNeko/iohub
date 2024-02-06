// File:     src/Poll.h
// Author:   AkashiNeko
// Project:  iohub
// Github:   https://github.com/AkashiNeko/iohub/

/* Copyright AkashiNeko. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 *
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
    std::vector<size_t> fd_map_;
    std::vector<pollfd> pollfd_arr_;
    bool is_open_;

public:
    Poll();
    virtual ~Poll() override = default;

    virtual void insert(int fd, int events) override;
    virtual void erase(int fd) override;
    virtual void modify(int fd, int events) override;
    virtual size_t size() const noexcept override;
    virtual void clear() noexcept override;

    virtual size_t wait(std::vector<fd_event_t>& fdevt_arr,
        int timeout = -1) override;

    virtual bool is_open() const noexcept override;
    virtual void close() noexcept override;

}; // class Poll

} // namespace iohub

#endif // IOHUB_POLL_H
