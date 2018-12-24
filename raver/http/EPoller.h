#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <raver/base/noncopyable.h>

namespace raver {

class Channel;
class IOManager;

class EPoller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;
  enum EPollEvent { ERROR = 0x00000001, READ = 0x00000002, WRITE = 0x00000004 };

  explicit EPoller(IOManager* iomanager);

  ~EPoller();

  void poll(ChannelList* active_channels);

  void updateChannel(Channel* channel);

  void removeChannel(Channel* channel);

  int fd() const {
    return epollfd_;
  }

 private:
  void fillActiveChannel(int num_events, ChannelList* active_channels) const;

  void update(int operation, Channel* channel);

 private:
  IOManager* iomanager_{nullptr};  // not own it.

  int epollfd_{-1};

  using EventList = std::vector<struct epoll_event>;
  EventList events_;

  using ChannelMap = std::unordered_map<int, Channel*>;
  ChannelMap channels_;

  static const int kInitEventListSize{16};
};

}  // namespace raver

#endif
