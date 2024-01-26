// Poll.cpp

#include "Poll.h"

namespace iohub {

Poll::Poll()
    : is_open_(true)
{
}

bool Poll::insert(int fd, int events) noexcept {
    if (!is_open_ || fd < 0 || !events || fd_map_.count(fd)) return false;
    fd_map_[fd] = events;
    pollfd pollfd_event = {fd, static_cast<short>(events), short(0)};
    pollfd_arr_.push_back(pollfd_event);
}

bool Poll::erase(int fd) noexcept {
}

bool Poll::modify(int fd, int events) noexcept {
}

int Poll::get_event(int fd) const noexcept {
}

size_t Poll::size() const noexcept {
}

void Poll::clear() noexcept {
}

fd_event_t Poll::wait(int timeout) {
}

bool Poll::is_open() const noexcept {
}

void Poll::close() noexcept {
}

} // namespace iohub
