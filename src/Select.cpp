// File:     src/Select.cpp
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

#include "Select.h"

namespace iohub {

Select::Select() : fd_hasharr_(32), max_(-1), size_(0),
        readsz_(0), writesz_(0), exceptsz_(0) {
    // clear fd_set
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&exceptfds_);
    is_open_ = true;
}

void Select::insert(int fd, int events) {
    // exceptions
    assert_throw_iohubexcept(is_open_, "[Select] insert(): Select is closed");
    assert_throw_iohubexcept(fd >= 0, "[Select] insert(): Invalid fd");
    assert_throw_iohubexcept(events, "[Select] insert(): Events is empty. "
        "If you want to remove fd from select, use Select::erase()");
    assert_throw_iohubexcept(!(events & ~0b111),
        "[Select] insert(): Events is not supported. "
        "Select supports only IOHUB_IN, IOHUB_OUT, IOHUB_PRI");

    if (fd >= fd_hasharr_.size())
        fd_hasharr_.resize(fd + 1);

    assert_throw_iohubexcept(!fd_hasharr_[fd],
        "[Select] insert(): The fd already exists. "
        "If you want to modify its event, use Select::modify()");

    // insert to the hash array
    if (size_++ == 0 || fd > max_) max_ = fd;
    fd_hasharr_[fd] = static_cast<unsigned char>(events);

    // set the fd_set
    if (events & IOHUB_IN) { FD_SET(fd, &readfds_); ++readsz_; }
    if (events & IOHUB_OUT) { FD_SET(fd, &writefds_); ++writesz_; }
    if (events & IOHUB_PRI) { FD_SET(fd, &exceptfds_); ++exceptsz_; }
}

void Select::erase(int fd) {
    // exceptions
    assert_throw_iohubexcept(is_open_, "[Select] erase(): Select is closed");
    assert_throw_iohubexcept(fd >= 0, "[Select] erase(): Invalid fd");
    assert_throw_iohubexcept(fd < fd_hasharr_.size() && fd_hasharr_[fd],
        "[Select] erase():The fd does not exist");

    // reset the fd_set
    unsigned char& old_events = fd_hasharr_[fd];
    if (old_events & IOHUB_IN) { FD_CLR(fd, &readfds_); --readsz_; }
    if (old_events & IOHUB_OUT) { FD_CLR(fd, &writefds_); --writesz_; }
    if (old_events & IOHUB_PRI) { FD_CLR(fd, &exceptfds_); --exceptsz_; }
    old_events = 0;

    // remove from the hash array
    if (--size_) {
        if (max_ == fd) for (; !fd_hasharr_[max_]; --max_);
    } else {
        max_ = -1;
    }
}

void Select::modify(int fd, int events) {
    // exceptions
    assert_throw_iohubexcept(is_open_, "[Select] modify(): Select is closed");
    assert_throw_iohubexcept(fd >= 0, "[Select] modify(): Invalid fd");
    assert_throw_iohubexcept(fd < fd_hasharr_.size() && fd_hasharr_[fd],
        "[Select] modify(): The fd does not exist");
    assert_throw_iohubexcept(events, "[Select] modify(): Events is empty. "
        "If you want to remove fd from select, use Select::erase()");
    assert_throw_iohubexcept(!(events & ~0b111),
        "[Select] modify(): Events is not supported. "
        "Select supports only IOHUB_IN, IOHUB_OUT, IOHUB_PRI");

    // update the number of fds
    unsigned char& old_events = fd_hasharr_[fd];
    int d_read = 0, d_write = 0, d_except = 0;
    readsz_ += d_read =
        ((int)!!(events & IOHUB_IN) - !!(old_events & IOHUB_IN));
    writesz_ += d_write =
        ((int)!!(events & IOHUB_OUT) - !!(old_events & IOHUB_OUT));
    exceptsz_ += d_except =
        ((int)!!(events & IOHUB_PRI) - !!(old_events & IOHUB_PRI));

    // update the fd_set
    if (d_read == 1) FD_SET(fd, &readfds_);
    else if (d_read == -1) FD_CLR(fd, &readfds_);
    if (d_write == 1) FD_SET(fd, &writefds_);
    else if (d_write == -1) FD_CLR(fd, &writefds_);
    if (d_except == 1) FD_SET(fd, &exceptfds_);
    else if (d_except == -1) FD_CLR(fd, &exceptfds_);

    // update the hash array
    old_events = events;
}

size_t Select::size() const noexcept {
    // return number of fds
    return size_;
}

void Select::clear() noexcept {
    if (is_open_) {
        FD_ZERO(&readfds_);
        FD_ZERO(&writefds_);
        FD_ZERO(&exceptfds_);
        size_ = writesz_ = readsz_ = exceptsz_ = 0;
        event_queue_.clear();
    }
}

fd_event_t Select::wait(int timeout) {
    // exceptions
    assert_throw_iohubexcept(is_open_, "[Select] wait(): Select is closed");
    assert_throw_iohubexcept(size_ > 0, "[Select] wait(): Select is empty");

    // push queue
    if (event_queue_.empty()) {

        // set timeout
        timeval time{};
        timeval* ptime = nullptr;
        if (timeout != -1) {
            time.tv_sec = timeout / 1000;
            time.tv_usec = timeout % 1000 * 1000;
            ptime = &time;
        }

        // fd_set
        fd_set read, write, except;
        fd_set* pread = readsz_ ? &(read = readfds_) : nullptr;
        fd_set* pwrite = writesz_ ? &(write = writefds_) : nullptr;
        fd_set* pexcept = exceptsz_ ? &(except = exceptfds_) : nullptr;

        // call select()
        int ret = ::select(max_ + 1, pread, pwrite, pexcept, ptime);
        if (ret == 0) return fd_event_t(-1, 0); // only non-blocking
        assert_throw_iohubexcept(ret > 0, "[Select] wait(): ", LAST_ERROR);

        // iterate over the result set
        for (size_t fd = 0, cnt = 0; cnt < ret && fd <= max_; ++fd) {
            if (fd_hasharr_[fd]) {
                // ready
                int event = 0;
                if (pread && FD_ISSET(fd, pread))
                    event |= IOHUB_IN;
                if (pwrite && FD_ISSET(fd, pwrite))
                    event |= IOHUB_OUT;
                if (pexcept && FD_ISSET(fd, pexcept))
                    event |= IOHUB_PRI;
                // push
                if (event) {
                    event_queue_.push(fd, event);
                    ++cnt;
                }
            }
        }
    } // queue is empty

    return event_queue_.pop();
}

bool Select::is_open() const noexcept {
    return is_open_;
}

void Select::close() noexcept {
    if (is_open_) {
        this->clear();
        is_open_ = false;
    }
}

} // namespace iohub
