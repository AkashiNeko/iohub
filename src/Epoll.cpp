// File:     src/Epoll.cpp
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

#include "Epoll.h"

namespace iohub {

namespace {
const size_t EPOLL_WAIT_BUFSIZE = 16;
}

Epoll::Epoll() : epoll_fd_(epoll_create(1)) {
    assert_throw(epoll_fd_ >= 0,
        "[epoll] Epoll create failed, ", std::strerror(errno));
}

void Epoll::insert(int fd, int events) {
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] insert: Epoll is closed");
    assert_throw(fd >= 0, "[epoll] insert: Invalid fd");
    assert_throw(events, "[epoll] insert: Events is empty. If you want to remove fd from epoll, use Epoll::erase()");

    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    assert_throw(!ret, "[epoll] insert: ", std::strerror(errno));
}

void Epoll::erase(int fd) {
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] erase: Epoll is closed");
    assert_throw(fd >= 0, "[epoll] erase: Invalid fd");

    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    assert_throw(!ret, "[epoll] erase: ", std::strerror(errno));
}

void Epoll::modify(int fd, int events) { 
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] modify: Epoll is closed");
    assert_throw(fd >= 0, "[epoll] modify: Invalid fd"); 
    assert_throw(events, "[epoll] modify: Events is empty. If you want to remove fd from epoll, use Epoll::erase()");

    // modify the fd event from epoll
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
    assert_throw(!ret, "[epoll] modify: ", std::strerror(errno));
}

size_t Epoll::size() const noexcept {
    // return number of fds
    return size_;
}

void Epoll::clear() noexcept {
    event_queue_.clear();
}

fd_event_t Epoll::wait(int timeout) {
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] wait: Epoll is closed");
    assert_throw(size_, "[epoll] wait: Epoll is empty");

    // push queue
    if (event_queue_.empty()) {

        // results
        epoll_event event_arr[EPOLL_WAIT_BUFSIZE]{};

        // epoll_wait
        int ret = epoll_wait(epoll_fd_, event_arr, EPOLL_WAIT_BUFSIZE, timeout);
        if (ret == 0) return fd_event_t(-1, 0); // only non-blocking
        assert_throw(ret > 0, "[epoll] wait: ", std::strerror(errno));

        // push
        for (size_t i = 0; i < ret; ++i) {
            event_queue_.push(fd_event_t(static_cast<int>(event_arr[i].data.fd),
                static_cast<int>(event_arr[i].events)));
        }
    } // queue is empty

    return event_queue_.pop();
}

bool Epoll::is_open() const noexcept {
    return epoll_fd_ != -1;
}

void Epoll::close() noexcept {
    if (epoll_fd_ != -1) {
        this->clear();
        ::close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

} // namespace iohub
