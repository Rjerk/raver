#include "Poller.h"
#include "../base/Logger.h"
#include "../base/Utils.h"
#include <sys/epoll.h>
#include <unistd.h>

namespace raver {

namespace detail {

constexpr int MAX_FDS_PER_POLL = 1024;

}

struct Poller::InternalPoller {
    InternalPoller(): fd_(-1) { }

    int fd_;
    struct epoll_event events_[detail::MAX_FDS_PER_POLL];
};

Poller::Poller()
    : poller_(new InternalPoller())
{
    LOG_TRACE << "Poller ctor";
}

Poller::~Poller()
{
    LOG_TRACE << "Poller dtor";
    if (poller_->fd_ >= 0) {
        ::close(poller_->fd_);
    }
}

void Poller::create()
{
    // FIXME: epoll_create1(EPOLL_CLOEXEC)
    poller_->fd_ = wrapper::epoll_create(detail::MAX_FDS_PER_POLL);
}

int Poller::poll()
{
    int ret;
    for ( ; ; ) {
        if ((ret = ::epoll_wait(poller_->fd_, poller_->events_,
                         detail::MAX_FDS_PER_POLL, 100)) >= 0) {
            break; // got event.
        } else if (errno == EINTR) {
            continue;
        } else {
            LOG_SYSFATAL << "epoll_wait";
        }
    }
    return ret;
}

void Poller::setEvent(int fd, Channel* data)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLPRI |
                EPOLLERR | EPOLLHUP | EPOLLET;
    ev.data.ptr = reinterpret_cast<void*>(data);
    if (::epoll_ctl(poller_->fd_, EPOLL_CTL_ADD, fd, &ev) != 0) {
        LOG_SYSERR << "Cannot add epoll descriptor";
    }
}

void Poller::getEvent(int i, int* events, Channel** data)
{
    *events &= 0x00000000;
    *data = reinterpret_cast<Channel*>(poller_->events_[i].data.ptr);
    auto flags = poller_->events_[i].events;
    if ((flags & EPOLLHUP) && !(flags & EPOLLIN)) {
        LOG_WARN << "fd: " << poller_->fd_ << " EPOLLHUP";
    }

    if (flags & EPOLLERR) {
        *events |= PollEvent::ERROR;
        return ;
    }

    if (flags & (EPOLLIN | EPOLLHUP)) {
        *events |= PollEvent::READ;
    }
    if (flags & (EPOLLOUT | EPOLLHUP)) {
        *events |= PollEvent::WRITE;
    }
}

}
