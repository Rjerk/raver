#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/epoll.h>
#include <functional>
#include "../base/noncopyable.h"

namespace raver {

class IOManager;

class Channel : noncopyable {
 public:
  using ReadEventCallback = std::function<void()>;
  using EventCallback = std::function<void()>;

  Channel(IOManager* iomanager, int fd);

  ~Channel();

  void handleEvent();

  void setReadCallback(const ReadEventCallback& cb) { readcb_ = cb; }
  void setWriteCallback(const EventCallback& cb) { writecb_ = cb; }
  void setCloseCallback(const EventCallback& cb) { closecb_ = cb; }
  void setErrorCallback(const EventCallback& cb) { errorcb_ = cb; }

  void setReadCallback(ReadEventCallback&& cb) { readcb_ = std::move(cb); }
  void setWriteCallback(EventCallback&& cb) { writecb_ = std::move(cb); }
  void setCloseCallback(EventCallback&& cb) { closecb_ = std::move(cb); }
  void setErrorCallback(EventCallback&& cb) { errorcb_ = std::move(cb); }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isReading() const { return events_ & kReadEvent; }
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  // used by epoller.
  int events() { return events_; }
  void setREvents(int revents) { revents_ = revents; }
  int index() const { return index_; }
  void setIndex(int index) { index_ = index; }

  void remove();

  int fd() const { return fd_; }

 private:
  void update();

 private:
  static const int kNoneEvent = 0;
  static const int kReadEvent = EPOLLIN | EPOLLPRI;
  static const int kWriteEvent = EPOLLOUT;

  IOManager* iomanager_;
  int fd_;
  int events_;
  int revents_;  // received event types of epoll.
  int index_;    // used by epoller;

  bool event_handling_;

  ReadEventCallback readcb_;
  EventCallback writecb_;
  EventCallback closecb_;
  EventCallback errorcb_;
};

}  // namespace raver

#endif
