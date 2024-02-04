// File:     src/Poll.cpp
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

#include "Poll.h"

namespace iohub {

Poll::Poll() : is_open_(true) {}

void Poll::insert(int fd, int events) {
    // exceptions
    assert_throw_iohubexcept(is_open_,
        "[Poll] insert(): Poll is closed");
    assert_throw_iohubexcept(fd >= 0,
        "[Poll] insert(): Invalid fd");
    assert_throw_iohubexcept(events,
        "[Poll] insert(): Events is empty. "
        "If you want to remove fd from poll, "
        "use Poll::erase()");
    assert_throw_iohubexcept(fd < fd_map_.size() && fd_map_[fd] == -1,
        "[Poll] insert(): The fd already exists. "
        "If you want to modify its event, "
        "use Poll::modify()");
    if (fd >= fd_map_.size()) fd_map_.resize(fd + 1, -1);

    // insert to fd map
    fd_map_[fd] = pollfd_arr_.size();

    // insert to pollfd list
    pollfd_arr_.push_back({fd, static_cast<short>(events), short(0)});
}

void Poll::erase(int fd) {
    // exceptions
    assert_throw_iohubexcept(is_open_,
        "[Poll] erase(): Poll is closed");
    assert_throw_iohubexcept(fd >= 0,
        "[Poll] erase(): Invalid fd");
    assert_throw_iohubexcept(fd < fd_map_.size() && fd_map_[fd] != -1,
        "[Poll] erase(): The fd does not exist");

    // remove fd from the fd map
    size_t index = fd_map_[fd];
    fd_map_[fd] = -1;

    // remove fd from the pollfd list
    if (index != pollfd_arr_.size() - 1) {
        pollfd_arr_[index] = std::move(pollfd_arr_.back());
    }
    pollfd_arr_.pop_back();
    event_queue_.erase(fd);
}

void Poll::modify(int fd, int events) {
    // exceptions
    assert_throw_iohubexcept(is_open_,
        "[Poll] modify(): Poll is closed");
    assert_throw_iohubexcept(fd >= 0,
        "[Poll] modify(): Invalid fd");
    assert_throw_iohubexcept(events,
        "[Poll] modify(): Events is empty. "
        "If you want to remove fd from poll, "
        "use Poll::erase()");
    assert_throw_iohubexcept(fd < fd_map_.size() && fd_map_[fd] != -1,
        "[Poll] modify(): The fd does not exist");

    // update the events
    pollfd_arr_[fd_map_[fd]].events = events;
}

size_t Poll::size() const noexcept {
    // return number of fds
    return pollfd_arr_.size();
}

void Poll::clear() noexcept {
    pollfd_arr_.clear();
    fd_map_.clear();
    event_queue_.clear();
}

fd_event_t Poll::wait(int timeout) {
    // exceptions
    assert_throw_iohubexcept(is_open_,
        "[Poll] wait(): Poll is closed");
    assert_throw_iohubexcept(!pollfd_arr_.empty(),
        "[Poll] wait(): Poll is empty");

    // push queue
    if (event_queue_.empty()) {

        // call poll api
        int ret = poll(pollfd_arr_.data(), pollfd_arr_.size(), timeout);
        if (ret == 0) return fd_event_t(-1, 0); // only non-blocking
        assert_throw_iohubexcept(ret > 0, "[Poll] wait(): ", LAST_ERROR);

        // iterate over the result set
        for (size_t i = 0, cnt = 0; cnt < ret; ++i) {
            pollfd& fd_event = pollfd_arr_[i];
            if (fd_event.revents) {
                event_queue_.push(fd_event.fd, fd_event.revents);
                fd_event.revents = 0;
                ++cnt;
            }
        }
    } // queue is empty

    return event_queue_.pop();
}

bool Poll::is_open() const noexcept {
    return is_open_;
}

void Poll::close() noexcept {
    if (is_open_) {
        this->clear();
        is_open_ = false;
    }
}

} // namespace iohub
