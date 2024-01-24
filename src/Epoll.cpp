// Epoll.cpp

#include "Epoll.h"

namespace iohub {

namespace {
const size_t EPOLL_WAIT_BUFSIZE = 16;
}

Epoll::Epoll() : epoll_fd_(epoll_create(1)) {
    assert_throw(epoll_fd_ >= 0, "[epoll] epoll create failed");
}

void Epoll::close() {
    ::close(epoll_fd_);
}

bool Epoll::insert(int fd, int events) {
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    return ret != -1;
}

bool Epoll::erase(int fd) {
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    return ret != -1;
}

bool Epoll::modify(int fd, int events) {
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
    return ret != -1;
}

EpollEvent Epoll::wait(int timeout) {
    epoll_event event_arr[EPOLL_WAIT_BUFSIZE]{};
    if (!event_queue_.empty()) timeout = 0;
    int ret = epoll_wait(epoll_fd_, event_arr, EPOLL_WAIT_BUFSIZE, timeout);
    assert_throw(ret >= 0, "[epoll] wait failed");
    for (size_t i = 0; i < ret; ++i) {
        event_queue_.push(EpollEvent((int)event_arr[i].data.fd, (uint32_t)event_arr[i].events));
    }
    if (event_queue_.empty()) return EpollEvent(-1, 0);
    EpollEvent result = event_queue_.front();
    event_queue_.pop();
    return result;
}

} // namespace iohub
