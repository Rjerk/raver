#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>
#include <memory>
#include "../base/noncopyable.h"

namespace raver {

class IOManager;
class Channel;

using AcceptorCallback = std::function<void(int)>;

class Acceptor {
 public:
  Acceptor(IOManager* iomanager, uint16_t port, AcceptorCallback acceptor_cb);

  ~Acceptor();

  void listen();

 private:
  void doAccept();

 private:
  IOManager* iomanager_;  // not own it.
  int listenfd_;

  std::unique_ptr<Channel> channel_;
  AcceptorCallback acceptor_cb_;
};

}  // namespace raver

#endif
