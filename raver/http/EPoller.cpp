#include <sys/epoll.h>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <vector>

#include <raver/base/Logger.h>
#include <raver/base/Utils.h>
#include <raver/http/EPoller.h>
#include <raver/http/Channel.h>

namespace raver {

namespace {

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

}  // namespace

EPoller::EPoller(IOManager* iomanager)
    : iomanager_{iomanager},
      epollfd_{::epoll_create1(EPOLL_CLOEXEC)},
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_SYSFATAL << "epoll_create1 error ";
  }
  LOG_TRACE << "Poller ctor";
}

EPoller::~EPoller() {
  LOG_TRACE << "Poller dtor";
  ::close(epollfd_);
}

void EPoller::poll(ChannelList* active_channels) {
  int num_events;
  if ((num_events = ::epoll_wait(epollfd_, events_.data(),
                                 static_cast<int>(events_.size()), 100)) > 0) {
    LOG_TRACE << "got events num: " << num_events;

    fillActiveChannel(num_events, active_channels);

    if (static_cast<size_t>(num_events) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (num_events == 0) {
    LOG_TRACE << "nothing happended.";
    return;
  } else if (num_events < 0) {
    if (errno != EINTR) {
      LOG_SYSERR << "epoll_wait error.";
    }
  }
}

void EPoller::fillActiveChannel(int num_events,
                                ChannelList* active_channels) const {
  LOG_TRACE << "fillActiveChannel";
  assert(static_cast<size_t>(num_events) <= events_.size());

  for (int i = 0; i < num_events; ++i) {
    auto channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->setREvents(events_[i].events);
    active_channels->push_back(channel);
  }
}

void EPoller::updateChannel(Channel* channel) {
  const int index = channel->index();

  LOG_TRACE << "index = " << index << " fd = " << channel->fd();

  if (int fd = channel->fd(); index == kNew || index == kDeleted) {
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    assert(index == kAdded);
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);

    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPoller::removeChannel(Channel* channel) {
  int fd = channel->fd();
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);

  [[ maybe_unused ]] size_t n = channels_.erase(fd);
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

void EPoller::update(int operation, Channel* channel) {
  struct epoll_event ev;
  ::bzero(&ev, sizeof(ev));
  ev.events = channel->events();
  ev.data.ptr = channel;

  LOG_TRACE << (operation == EPOLL_CTL_MOD ? "mod" : "add") << " read "
            << (ev.events & EPOLLIN) << " write " << (ev.events & EPOLLOUT);

  if (::epoll_ctl(epollfd_, operation, channel->fd(), &ev) < 0) {
    LOG_SYSERR << "epoll_ctl error";
  }
}

}  // namespace raver
