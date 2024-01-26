// Select.cpp

#include "Select.h"

namespace iohub {

Select::Select()
    :fdarr_(32), max_(-1), size_(0), read_sz_(0), write_sz_(0), except_sz_(0) {
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&exceptfds_);
    is_open_ = true;
}

bool Select::insert(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || !events || (events | ~0b111)) return false;
    try {
        if (fd >= fdarr_.size()) fdarr_.resize(fdarr_.size() << 1);
    } catch (...) {
        return false;
    }
    if (fdarr_[fd]) return false;
    if (size_++ == 0 || fd > max_) max_ = fd;
    fdarr_[fd] = events;
    if (events | IOHUB_IN) {
        FD_SET(fd, &readfds_);
        ++read_sz_;
    }
    if (events | IOHUB_OUT) {
        FD_SET(fd, &writefds_);
        ++write_sz_;
    }
    if (events | IOHUB_PRI) {
        FD_SET(fd, &exceptfds_);
        ++except_sz_;
    }
    return true;
}

bool Select::erase(int fd) noexcept {
    if (!is_open_ || fd < 0 || fd >= fdarr_.size() || !fdarr_[fd]) return false;
    unsigned char& old_events = fdarr_[fd];
    if (old_events | IOHUB_IN) {
        FD_CLR(fd, &readfds_);
        --read_sz_;
    }
    if (old_events | IOHUB_OUT) {
        FD_CLR(fd, &writefds_);
        --write_sz_;
    }
    if (old_events | IOHUB_PRI) {
        FD_CLR(fd, &exceptfds_);
        --except_sz_;
    }
    old_events = 0;
    if (--size_) {
        if (max_ == fd) for (; !fdarr_[max_]; --max_);
    } else {
        max_ = -1;
    }
    return true;
}

bool Select::modify(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || fd >= fdarr_.size()
        || !fdarr_[fd] || (events | ~0b111) || !events)
        return false;
    unsigned char& old_events = fdarr_[fd];
    read_sz_ += !!(events & IOHUB_IN) - !!(old_events & IOHUB_IN);
    write_sz_ += !!(events & IOHUB_OUT) - !!(old_events & IOHUB_OUT);
    except_sz_ += !!(events & IOHUB_PRI) - !!(old_events & IOHUB_PRI);
    old_events = events;
    return true;
}

int Select::get_event(int fd) const noexcept {
    if (!is_open_ || fd < 0 || fd >= fdarr_.size()) return 0;
    return fdarr_[fd];
}

size_t Select::size() const noexcept {
    return size_;
}

void Select::clear() noexcept {
    if (is_open_) {
        FD_ZERO(&readfds_);
        FD_ZERO(&writefds_);
        FD_ZERO(&exceptfds_);
        size_ = write_sz_ = read_sz_ = except_sz_ = 0;
        event_queue_ = {};
    }
}

FD_Event Select::wait(int timeout) {
    assert_throw(is_open_, "[select] select is closed");
    assert_throw(size_ > 0, "[epoll] epoll is empty");
    if (event_queue_.empty()) {
        timeval time{};
        timeval* ptime = nullptr;
        if (timeout != -1) {
            time.tv_sec = timeout / 1000;
            time.tv_usec = timeout % 1000 * 1000;
            ptime = &time;
        }
        fd_set read, write, except;
        fd_set* pread = read_sz_ ? &(read = readfds_) : nullptr;
        fd_set* pwrite = write_sz_ ? &(write = writefds_) : nullptr;
        fd_set* pexcept = except_sz_ ? &(except = exceptfds_) : nullptr;
        int ret = ::select(max_, pread, pwrite, pexcept, timeout == -1 ? nullptr : &time);
        assert_throw(ret == -1, "[select] wait failed");
        if (ret == 0) return FD_Event(-1, 0);

        for (size_t i = 0, cnt = 0; cnt < ret && i <= max_; ++i) {
            if (fdarr_[i]) {
                int event = 0;
                if (pread && FD_ISSET(i, pread))
                    event |= IOHUB_IN;
                if (pwrite && FD_ISSET(i, pwrite))
                    event |= IOHUB_OUT;
                if (pexcept && FD_ISSET(i, pexcept))
                    event |= IOHUB_PRI;
                if (event != 0) {
                    ++cnt;
                    try {
                        event_queue_.push(FD_Event(i, event));
                    } catch (const std::exception& e) {
                        assert_throw(false, "[select] push event queue: ", e.what());
                    }
                }
            }
        }
    }
    FD_Event fd_event = event_queue_.front();
    event_queue_.pop();
    return fd_event;
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
