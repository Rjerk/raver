#include "Poller.h"
#include "Logger.h"
#include <sys/epoll.h>
#include <unistd.h>

namespace raver {

namespace {

static constexpr int MAX_FDS_PER_POLL = 1024;

}
struct Poller::InternalPoller {
    InternalPoller(): fd_(-1) { }

    int fd_;
    struct epoll_event events_[MAX_FDS_PER_POLL];
};

Poller::Poller()
    : poller_(new InternalPoller())
{
}

Poller::~Poller()
{
    if (poller_->fd_ >= 0) {
        ::close(poller_->fd_);
    }
    delete poller_;
}

void Poller::create()
{
    if ((poller_->fd_ = ::epoll_create(MAX_FDS_PER_POLL)) < 0) {
        LOG_ERROR << "epoll_create";
    }
}

int Poller::poll()
{
    int ret;
    for ( ; ; ) {
        if ((ret = ::epoll_wait(poller_->fd_, poller_->events_,
                         MAX_FDS_PER_POLL, 100)) >= 0) {
            break; // got event.
        } else if (errno == EINTR) {
            continue;
        } else {
            LOG_ERROR << "epoll_wait";
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
        LOG_ERROR << "Cannot add epoll descriptor";
    }
}

void Poller::getEvent(int i, int* events, Channel** data)
{
    *events &= 0x00000000;
    *data = reinterpret_cast<Channel*>(poller_->events_[i].data.ptr);
    if (poller_->events_[i].events & EPOLLERR) {
        *events |= PollEvent::ERROR;
    }
    if (poller_->events_[i].events & (EPOLLHUP | EPOLLIN)) {
        *events |= PollEvent::READ;
    }
    if (poller_->events_[i].events & (EPOLLHUP | EPOLLOUT)) {
        *events |= PollEvent::WRITE;
    }
}

}
