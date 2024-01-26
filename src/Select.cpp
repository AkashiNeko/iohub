// Select.cpp

#include "Select.h"

namespace iohub {

Select::Select()
    : fd_arr_(32)
    , max_(-1)
    , size_(0)
    , readsz_(0)
    , writesz_(0)
    , exceptsz_(0)
{
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&exceptfds_);
    is_open_ = true;
}

bool Select::insert(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || !events || (events & ~0b111)) return false;
    try {
        if (fd >= fd_arr_.size()) fd_arr_.resize(fd_arr_.size() << 1);
    } catch (...) {
        return false;
    }
    if (fd_arr_[fd]) return false;
    if (size_++ == 0 || fd > max_) max_ = fd;
    fd_arr_[fd] = events;
    if (events & IOHUB_IN) {
        FD_SET(fd, &readfds_);
        ++readsz_;
    }
    if (events & IOHUB_OUT) {
        FD_SET(fd, &writefds_);
        ++writesz_;
    }
    if (events & IOHUB_PRI) {
        FD_SET(fd, &exceptfds_);
        ++exceptsz_;
    }
    return true;
}

bool Select::erase(int fd) noexcept {
    if (!is_open_ || fd < 0 || fd >= fd_arr_.size() || !fd_arr_[fd]) return false;
    unsigned char& old_events = fd_arr_[fd];
    if (old_events & IOHUB_IN) {
        FD_CLR(fd, &readfds_);
        --readsz_;
    }
    if (old_events & IOHUB_OUT) {
        FD_CLR(fd, &writefds_);
        --writesz_;
    }
    if (old_events & IOHUB_PRI) {
        FD_CLR(fd, &exceptfds_);
        --exceptsz_;
    }
    old_events = 0;
    if (--size_) {
        if (max_ == fd) for (; !fd_arr_[max_]; --max_);
    } else {
        max_ = -1;
    }
    return true;
}

bool Select::modify(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || fd >= fd_arr_.size()
        || !fd_arr_[fd] || (events & ~0b111) || !events)
        return false;
    unsigned char& old_events = fd_arr_[fd];
    readsz_ += !!(events & IOHUB_IN) - !!(old_events & IOHUB_IN);
    writesz_ += !!(events & IOHUB_OUT) - !!(old_events & IOHUB_OUT);
    exceptsz_ += !!(events & IOHUB_PRI) - !!(old_events & IOHUB_PRI);
    old_events = events;
    return true;
}

int Select::get_event(int fd) const noexcept {
    if (!is_open_ || fd < 0 || fd >= fd_arr_.size()) return 0;
    return fd_arr_[fd];
}

size_t Select::size() const noexcept {
    return size_;
}

void Select::clear() noexcept {
    if (is_open_) {
        FD_ZERO(&readfds_);
        FD_ZERO(&writefds_);
        FD_ZERO(&exceptfds_);
        size_ = writesz_ = readsz_ = exceptsz_ = 0;
        event_queue_ = {};
    }
}

fd_event_t Select::wait(int timeout) {
    assert_throw(is_open_, "[select] select is closed");
    assert_throw(size_ > 0, "[select] select is empty");
    if (event_queue_.empty()) {
        timeval time{};
        timeval* ptime = nullptr;
        if (timeout != -1) {
            time.tv_sec = timeout / 1000;
            time.tv_usec = timeout % 1000 * 1000;
            ptime = &time;
        }
        fd_set read, write, except;
        fd_set* pread = readsz_ ? &(read = readfds_) : nullptr;
        fd_set* pwrite = writesz_ ? &(write = writefds_) : nullptr;
        fd_set* pexcept = exceptsz_ ? &(except = exceptfds_) : nullptr;
        int ret = ::select(max_ + 1, pread, pwrite, pexcept, timeout == -1 ? nullptr : &time);
        assert_throw(ret >= 0, "[select] wait failed");
        if (ret == 0) return fd_event_t(-1, 0);

        for (size_t i = 0, cnt = 0; cnt < ret && i <= max_; ++i) {
            if (fd_arr_[i]) {
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
                        event_queue_.push(fd_event_t(i, event));
                    } catch (const std::exception& e) {
                        assert_throw(false, "[select] push event queue: ", e.what());
                    }
                }
            }
        }
    }
    fd_event_t fd_event = event_queue_.front();
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
