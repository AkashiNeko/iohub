// Poll.cpp

#include "Poll.h"

namespace iohub {

Poll::Poll()
    : is_open_(true)
{
}

bool Poll::insert(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || !events || fd_map_.count(fd)) return false;
    pollfd pollfd_event = {fd, static_cast<short>(events), short(0)};
    fd_map_[fd] = pollfd_arr_.size();
    pollfd_arr_.push_back(pollfd_event);
    return true;
}

bool Poll::erase(int fd) noexcept {
    if (!is_open_ || !fd_map_.count(fd)) return false;
    size_t index = fd_map_[fd];
    fd_map_.erase(fd);
    if (index != pollfd_arr_.size() - 1) {
        pollfd_arr_[index] = std::move(pollfd_arr_.back());
    }
    pollfd_arr_.pop_back();
    return true;
}

bool Poll::modify(int fd, int events) noexcept {
    if (!is_open_ || !events || !fd_map_.count(fd)) return false;
    pollfd_arr_[fd_map_[fd]].events = events;
    return true;
}

int Poll::get_event(int fd) const noexcept {
    if (!is_open_ || !fd_map_.count(fd)) return 0;
    return pollfd_arr_[fd_map_.at(fd)].events;
}

size_t Poll::size() const noexcept {
    return pollfd_arr_.size();
}

void Poll::clear() noexcept {
    pollfd_arr_.clear();
    fd_map_.clear();
    event_queue_ = {};
}

fd_event_t Poll::wait(int timeout) {
    assert_throw(is_open_, "[poll] poll is closed");
    if (event_queue_.empty()) {
        assert_throw(!pollfd_arr_.empty(), "[poll] poll is empty");
        int ret = poll(pollfd_arr_.data(), pollfd_arr_.size(), timeout);
        if (ret == 0) return fd_event_t(-1, 0);
        assert_throw(ret > 0, "[poll] wait failed");
        for (size_t i = 0, cnt = 0; cnt < ret; ++i) {
            if (pollfd_arr_[i].revents) {
                event_queue_.push(fd_event_t(i, pollfd_arr_[i].revents));
                ++cnt;
            }
        }
    }
    fd_event_t result = event_queue_.front();
    event_queue_.pop();
    return result;
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
