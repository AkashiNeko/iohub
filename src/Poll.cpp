// Poll.cpp

#include "Poll.h"

namespace iohub {

Poll::Poll() {
}

bool Poll::insert(int fd, int events) noexcept {
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

FD_Event Poll::wait(int timeout) {
}

bool Poll::is_open() const noexcept {
}

void Poll::close() noexcept {
}

} // namespace iohub
