// Select.cpp

#include "Select.h"

namespace iohub {

Select::Select() {
}

bool Select::insert(int fd, int events) noexcept {
}

bool Select::erase(int fd) noexcept {
}

bool Select::modify(int fd, int events) noexcept {
}

int Select::get_event(int fd) const noexcept {
}

size_t Select::size() const noexcept {
}

void Select::clear() noexcept {
}

FD_Event Select::wait(int timeout) {
}

bool Select::is_open() const noexcept {
}

void Select::close() noexcept {
}

} // namespace iohub
