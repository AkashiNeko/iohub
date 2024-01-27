// Epoll.cpp

#include "Epoll.h"

namespace iohub {

namespace {
const size_t EPOLL_WAIT_BUFSIZE = 16;
}

Epoll::Epoll()
    : epoll_fd_(epoll_create(1))    // create epoll
{
    assert_throw(epoll_fd_ >= 0, "[epoll] Epoll create failed, ", std::strerror(errno));
}

void Epoll::insert(int fd, int events) {
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] insert: Epoll is closed");
    assert_throw(fd >= 0, "[epoll] insert: Invalid fd");
    assert_throw(events, "[epoll] insert: Events is empty. If you want to remove fd from epoll, use Epoll::erase()");
    assert_throw(!fd_map_.count(fd), "[epoll] insert: The fd already exists. If you want to modify its event, use Epoll::modify()");

    // insert to fd map
    fd_map_[fd] = events;

    // add fd event to epoll
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    assert_throw(!ret, "[epoll] insert: ", std::strerror(errno));
}

void Epoll::erase(int fd) {
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] erase: Epoll is closed");
    assert_throw(fd >= 0, "[epoll] erase: Invalid fd");
    auto it = fd_map_.find(fd);
    assert_throw(it != fd_map_.end(), "[epoll] erase: The fd does not exist");

    // remove fd from the fd map
    fd_map_.erase(it);

    // delete fd event from epoll
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    assert_throw(!ret, "[epoll] erase: ", std::strerror(errno));
}

void Epoll::modify(int fd, int events) {
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] modify: Epoll is closed");
    assert_throw(fd >= 0, "[epoll] modify: Invalid fd");
    assert_throw(events, "[epoll] modify: Events is empty. If you want to remove fd from epoll, use Epoll::erase()");
    auto it = fd_map_.find(fd);
    assert_throw(it != fd_map_.end(), "[epoll] modify: The fd does not exist");

    // update the event from fd map
    it->second = events;

    // modify the fd event from epoll
    epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
    assert_throw(!ret, "[epoll] modify: ", std::strerror(errno));
}

int Epoll::get_event(int fd) const noexcept {
    // return 0 if fd does not exist, otherwise return an event
    auto it = fd_map_.find(fd);
    return it == fd_map_.end() ? 0 : it->second;
}

size_t Epoll::size() const noexcept {
    // return number of fds
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
    // exceptions
    assert_throw(epoll_fd_ != -1, "[epoll] wait: Epoll is closed");
    assert_throw(!fd_map_.empty(), "[epoll] wait: Epoll is empty");

    // push queue
    if (event_queue_.empty()) {

        // results
        epoll_event event_arr[EPOLL_WAIT_BUFSIZE]{};

        // epoll_wait
        int ret = epoll_wait(epoll_fd_, event_arr, EPOLL_WAIT_BUFSIZE, timeout);
        if (ret == 0) return fd_event_t(-1, 0); // only non-blocking
        assert_throw(ret > 0, "[epoll] wait: ", std::strerror(errno));

        // push
        for (size_t i = 0; i < ret; ++i) {
            event_queue_.push(fd_event_t(static_cast<int>(event_arr[i].data.fd),
                static_cast<int>(event_arr[i].events)));
        }
    } // queue is empty

    // return front of queue
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
