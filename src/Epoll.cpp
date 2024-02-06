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
const size_t EPOLL_WAIT_BUFSIZE = 4096;
}

Epoll::Epoll() : epoll_fd_(epoll_create(1)), size_(0),
        event_arr_(new epoll_event[EPOLL_WAIT_BUFSIZE]) {
    assert_throw_iohubexcept(epoll_fd_ >= 0,
        "[Epoll] Epoll create failed, ", LAST_ERROR);
}

void Epoll::insert(int fd, int events) {
    // exceptions
    assert_throw_iohubexcept(epoll_fd_ != -1,
        "[Epoll] insert(): Epoll is closed");
    assert_throw_iohubexcept(fd >= 0,
        "[Epoll] insert(): Invalid fd");
    assert_throw_iohubexcept(events,
        "[Epoll] insert(): Events is empty. "
        "If you want to remove fd from epoll, "
        "use Epoll::erase()");

    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_,
        EPOLL_CTL_ADD, fd, &event);
    assert_throw_iohubexcept(ret == 0,
        "[Epoll] insert(): ", LAST_ERROR);
    ++size_;
}

void Epoll::erase(int fd) {
    // exceptions
    assert_throw_iohubexcept(epoll_fd_ != -1,
        "[Epoll] erase(): Epoll is closed");
    assert_throw_iohubexcept(fd >= 0,
        "[Epoll] erase(): Invalid fd");

    int ret = epoll_ctl(epoll_fd_,
        EPOLL_CTL_DEL, fd, nullptr);
    assert_throw_iohubexcept(ret == 0,
        "[Epoll] erase(): ", LAST_ERROR);
    --size_;
}

void Epoll::modify(int fd, int events) { 
    // exceptions
    assert_throw_iohubexcept(epoll_fd_ != -1,
        "[Epoll] modify(): Epoll is closed");
    assert_throw_iohubexcept(fd >= 0,
        "[Epoll] modify(): Invalid fd"); 
    assert_throw_iohubexcept(events,
        "[Epoll] modify(): Events is empty. "
        "If you want to remove fd from epoll, "
        "use Epoll::erase()");

    // modify the fd event from epoll
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_,
        EPOLL_CTL_MOD, fd, &event);
    assert_throw_iohubexcept(!ret,
        "[Epoll] modify(): ", LAST_ERROR);
}

size_t Epoll::size() const noexcept {
    // return number of fds
    return size_;
}

void Epoll::clear() noexcept {
    ::close(epoll_fd_);
    epoll_fd_ = epoll_create(1);
}

size_t Epoll::wait(std::vector<fd_event_t>& fdevt_arr, int timeout) {
    // exceptions
    assert_throw_iohubexcept(epoll_fd_ != -1,
        "[Epoll] wait(): Epoll is closed");
    assert_throw_iohubexcept(size_,
        "[Epoll] wait(): Epoll is empty");

    fdevt_arr.clear();
    int ret = 0, result = 0;
    do {
        ret = epoll_wait(epoll_fd_, event_arr_, EPOLL_WAIT_BUFSIZE, timeout);
        if (ret == 0) return result;
        timeout = 0;
        assert_throw_iohubexcept(ret > 0,
            "[Epoll] wait(): ", LAST_ERROR);
        // push
        fdevt_arr.resize(result + ret);
        for (size_t cnt = result; cnt < ret; ++cnt) {
            fdevt_arr[cnt++] = {static_cast<int>(event_arr_[cnt].data.fd),
                static_cast<int>(event_arr_[cnt].events)};
        }
        result += ret;
    } while (ret == EPOLL_WAIT_BUFSIZE);
    return result;
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
