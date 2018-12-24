#include <raver/base/Logger.h>
#include <raver/http/IOManager.h>
#include <raver/http/Channel.h>
//#include "../base/ThreadPool.h"

#include <cassert>

namespace raver {

Channel::Channel(IOManager* iomanager, int fd)
    : iomanager_{iomanager}, fd_{fd} {
  LOG_TRACE << "Channel ctor";
}

Channel::~Channel() {
  LOG_TRACE << "Channel dtor";
  assert(!event_handling_);
}

void Channel::handleEvent() {
  event_handling_ = true;

  LOG_TRACE << "handling revents...";

  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    if (closecb_) {
      closecb_();
    }
  }

  if (revents_ & EPOLLERR) {
    if (errorcb_) {
      errorcb_();
    }
  }

  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (readcb_) {
      readcb_();
    }
  }

  if (revents_ & EPOLLOUT) {
    if (writecb_) {
      writecb_();
    }
  }

  event_handling_ = false;
}

void Channel::remove() {
  LOG_TRACE << "remove channel: " << this;
  assert(isNoneEvent());
  iomanager_->removeChannel(this);
}

void Channel::update() {
  iomanager_->updateChannel(this);
}

}  // namespace raver
