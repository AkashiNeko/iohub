// Epoll.cpp

#include "Epoll.h"

namespace iohub {

namespace {
const size_t EPOLL_WAIT_BUFSIZE = 16;
}

Epoll::Epoll() : epoll_fd_(epoll_create(1)) {
    assert_throw(epoll_fd_ >= 0, "[epoll] epoll create failed");
}

bool Epoll::insert(int fd, int events) noexcept {
    if (epoll_fd_ == -1 || fd < 0 || !events || fd_map_.count(fd)) return false;
    fd_map_[fd] = events;
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    return !epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
}

bool Epoll::erase(int fd) noexcept {
    if (epoll_fd_ == -1 || fd < 0) return false;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return false;
    fd_map_.erase(it);
    return !epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

bool Epoll::modify(int fd, int events) noexcept {
    if (epoll_fd_ == -1 || fd < 0 || events == 0) return false;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return false;
    it->second = events;
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    return !epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
}

int Epoll::get_event(int fd) const noexcept {
    auto it = fd_map_.find(fd);
    return it == fd_map_.end() ? 0 : it->second;
}

size_t Epoll::size() const noexcept {
    return fd_map_.size();
}

void Epoll::clear() noexcept {
    for (auto fd_event : fd_map_) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd_event.first, nullptr);
    }
    fd_map_.clear();
    event_queue_ = {};
}

fd_event_t Epoll::wait(int timeout) {
    assert_throw(epoll_fd_ != -1, "[epoll] epoll is closed");
    assert_throw(fd_map_.size() > 0, "[epoll] epoll is empty");
    if (event_queue_.empty()) {
        epoll_event event_arr[EPOLL_WAIT_BUFSIZE]{};
        int ret = epoll_wait(epoll_fd_, event_arr, EPOLL_WAIT_BUFSIZE, timeout);
        assert_throw(ret >= 0, "[epoll] wait failed");
        if (ret == 0) return fd_event_t(-1, 0);
        for (size_t i = 0; i < ret; ++i) {
            try {
                event_queue_.push(fd_event_t(static_cast<int>(event_arr[i].data.fd),
                    static_cast<int>(event_arr[i].events)));
            } catch (const std::exception& e) {
                assert_throw(false, "[epoll] push event queue: ", e.what());
            }
        }
    }
    fd_event_t result = event_queue_.front();
    event_queue_.pop();
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
