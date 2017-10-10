#include "Poller.h"
#include "../base/Logger.h"
#include "../base/Utils.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

namespace raver {

namespace {

constexpr int EVENT_INIT_SIZE = 10;

}

struct Poller::InternalPoller {
    InternalPoller(): fd_(-1), events_(EVENT_INIT_SIZE){ }

    int fd_;
    std::vector<struct epoll_event> events_;
};

Poller::Poller()
    : poller_(new InternalPoller())
{
    poller_->fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (poller_->fd_ < 0) {
        LOG_SYSERR << "epoll_create1 error ";
    }
    LOG_TRACE << "Poller ctor";
}

Poller::~Poller()
{
    LOG_TRACE << "Poller dtor";
    if (poller_->fd_ >= 0) {
        ::close(poller_->fd_);
    }
}

int Poller::poll()
{
    int ret;
    for ( ; ; ) {
        if ((ret = ::epoll_wait(poller_->fd_, &*poller_->events_.begin(),
                            static_cast<int>(poller_->events_.size()), 100)) > 0) {
            LOG_TRACE << "got events num: " << ret;
            if (static_cast<size_t>(ret) == poller_->events_.size()) {
                poller_->events_.resize(poller_->events_.size() * 1.5);
            }
            break;
        } else if (ret == 0) {
            //LOG_TRACE << "nothing happended.";
            continue;
        } else if (ret < 0) {
            if (errno != EINTR) {
                LOG_SYSERR << "epoll_wait";
                break;
            }
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
