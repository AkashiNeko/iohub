// Poll.cpp

#include "Poll.h"

namespace iohub {

Poll::Poll() {
}

bool Poll::insert(int fd, int events) {
}

bool Poll::erase(int fd) {
}

bool Poll::modify(int fd, int events) {
}

int Poll::get_event(int fd) const {
}

size_t Poll::size() const {
}

FD_Event Poll::wait(int timeout) {
}

bool Poll::is_open() const {
}

void Poll::close() {
}

} // namespace iohub
