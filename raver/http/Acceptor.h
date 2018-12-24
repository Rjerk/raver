#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>
#include <memory>
#include <raver/base/noncopyable.h>

namespace raver {

class IOManager;
class Channel;

using AcceptorCallback = std::function<void(int)>;

class Acceptor {
 public:
  Acceptor(IOManager* iomanager, uint16_t port, AcceptorCallback acceptor_cb);

  ~Acceptor();

  void Listen();

 private:
  void DoAccept();

 private:
  IOManager* iomanager_{nullptr};  // not own it.

  AcceptorCallback acceptor_cb_;

  int listenfd_{-1};

  std::unique_ptr<Channel> channel_;
};

}  // namespace raver

#endif
