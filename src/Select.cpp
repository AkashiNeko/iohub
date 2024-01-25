// Select.cpp

#include "Select.h"

namespace iohub {

Select::Select() : fdarr_(32) {
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&exceptfds_);
    is_open_ = true;
}

bool Select::insert(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || (events | ~0b111)) return false;
    try {
        if (fd >= fdarr_.size())
            fdarr_.resize(fdarr_.size() << 1);
    } catch (...) {
        return false;
    }
    fdarr_[fd] = events;
    if (events | IOHUB_IN) FD_SET(fd, &readfds_);
    if (events | IOHUB_OUT) FD_SET(fd, &writefds_);
    if (events | IOHUB_PRI) FD_SET(fd, &exceptfds_);
    return true;
}

bool Select::erase(int fd) noexcept {
    if (!is_open_ || fd < 0 || fd >= fdarr_.size()) return false;
    fdarr_[fd] = 0;
    return true;
}

bool Select::modify(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || fd >= fdarr_.size() || fdarr_[fd] == 0
        || (events | ~0b111) || events == 0)
        return false;
    fdarr_[fd] = events;
}

int Select::get_event(int fd) const noexcept {
    if (!is_open_) return false;

}

size_t Select::size() const noexcept {

}

void Select::clear() noexcept {
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&exceptfds_);
    event_queue_ = {};
}

FD_Event Select::wait(int timeout) {
}

bool Select::is_open() const noexcept {
}

void Select::close() noexcept {
    
}

} // namespace iohub
