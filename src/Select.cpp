// Select.cpp

#include "Select.h"

namespace iohub {

Select::Select() {
}

bool Select::insert(int fd, int events) {
}

bool Select::erase(int fd) {
}

bool Select::modify(int fd, int events) {
}

int Select::get_event(int fd) const {
}

size_t Select::size() const {
}

FD_Event Select::wait(int timeout) {
}

bool Select::is_open() const {
}

void Select::close() {
}

} // namespace iohub
