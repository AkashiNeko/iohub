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

Poll::Poll()
    : is_open_(true)
{
}

void Poll::insert(int fd, int events) {
    // exceptions
    assert_throw(is_open_, "[poll] insert: Poll is closed");
    assert_throw(fd >= 0, "[poll] insert: Invalid fd");
    assert_throw(events, "[poll] insert: Events is empty. If you want to remove fd from poll, use Poll::erase()");
    assert_throw(!fd_map_.count(fd), "[poll] insert: The fd already exists. If you want to modify its event, use Poll::modify()");

    // insert to fd map
    fd_map_.insert({fd, pollfd_arr_.size()});

    // insert to pollfd list
    pollfd pollfd_event = {fd, static_cast<short>(events), short(0)};
    pollfd_arr_.push_back(pollfd_event);
}

void Poll::erase(int fd) {
    // exceptions
    assert_throw(is_open_, "[poll] erase: Poll is closed");
    assert_throw(fd >= 0, "[poll] erase: Invalid fd");
    assert_throw(fd_map_.count(fd), "[poll] erase: The fd does not exist");

    // remove fd from the fd map
    auto it = fd_map_.find(fd);
    size_t index = it->second;
    fd_map_.erase(it);

    // remove fd from the pollfd list
    if (index != pollfd_arr_.size() - 1) {
        pollfd_arr_[index] = std::move(pollfd_arr_.back());
    }
    pollfd_arr_.pop_back();
}

void Poll::modify(int fd, int events) {
    // exceptions
    assert_throw(is_open_, "[poll] modify: Poll is closed");
    assert_throw(fd >= 0, "[poll] modify: Invalid fd");
    assert_throw(events, "[poll] modify: Events is empty. If you want to remove fd from poll, use Poll::erase()");
    auto it = fd_map_.find(fd);
    assert_throw(it != fd_map_.end(), "[poll] modify: The fd does not exist");

    // update the events
    pollfd_arr_[it->second].events = events;
}

size_t Poll::size() const noexcept {
    // return number of fds
    return pollfd_arr_.size();
}

void Poll::clear() noexcept {
    pollfd_arr_.clear();
    fd_map_.clear();
    event_queue_ = {};
}

fd_event_t Poll::wait(int timeout) {
    // exceptions
    assert_throw(is_open_, "[poll] wait: Poll is closed");
    assert_throw(!pollfd_arr_.empty(), "[poll] wait: Poll is empty");

    // push queue
    if (event_queue_.empty()) {

        // poll
        int ret = poll(pollfd_arr_.data(), pollfd_arr_.size(), timeout);
        if (ret == 0) return fd_event_t(-1, 0); // only non-blocking
        assert_throw(ret > 0, "[poll] wait: ", std::strerror(errno));

        // iterate over the result set
        for (size_t i = 0, cnt = 0; cnt < ret; ++i) {
            pollfd& fd_event = pollfd_arr_[i];
            if (fd_event.revents) {
                event_queue_.push(fd_event_t(fd_event.fd, fd_event.revents));
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
