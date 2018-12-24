#include <raver/base/Logger.h>
#include <raver/base/Utils.h>
#include <raver/http/Channel.h>
#include <raver/http/IOManager.h>
#include <raver/http/Acceptor.h>

#include <cstring>
#include <memory>
#include <utility>

namespace raver {

Acceptor::Acceptor(IOManager* iomanager, uint16_t port,
                   AcceptorCallback acceptor_cb)
    : iomanager_{iomanager},
      acceptor_cb_{std::move(acceptor_cb)} {
  LOG_TRACE << "Acceptor ctor";

  listenfd_ = utils::socket(AF_INET, SOCK_STREAM, 0);

  utils::setNonBlockAndCloseOnExec(listenfd_);

  struct sockaddr_in serv_addr;
  ::bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  int opt = 1;
  ::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  ::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

  utils::bindOrDie(listenfd_, reinterpret_cast<struct sockaddr*>(&serv_addr));

  channel_ = std::make_unique<Channel>(iomanager_, listenfd_);
  channel_->setReadCallback(std::bind(&Acceptor::DoAccept, this));

  utils::listenOrDie(listenfd_);

  channel_->enableReading();
}

Acceptor::~Acceptor() {
  LOG_TRACE << "Acceptor dtor";
  channel_->disableAll();
  channel_->remove();
  utils::close(listenfd_);
}

void Acceptor::DoAccept() {
  LOG_TRACE << "start accept, listenfd: " << listenfd_;
  for (;;) {
    struct sockaddr_in clnt_addr;
    socklen_t len = sizeof(clnt_addr);
    int connfd =
        ::accept4(listenfd_, reinterpret_cast<struct sockaddr*>(&clnt_addr),
                  &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd == -1) {
      auto saved_errno = errno;
      switch (saved_errno) {
        case EINTR:  // interrupted by a signal that was caught before a valid
                     // connection arrived.
        case ECONNABORTED:  // a connection has been aborted.
          LOG_TRACE << "EINTR or ECONNABORTED in accept4";
          continue;

        case EAGAIN:  // same as EWOULDBLOCK, if no pending connections are
                      // present on the incomplete connection queue.
          LOG_TRACE << "got EAGAIN in accept4";
          break;

        case EBADF:
        case EFAULT:
        case EINVAL:
        case EMFILE:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
        case EPROTO:
        case EPERM:
          LOG_SYSERR << "unexpected error of accept.";
          break;

        default:  // various Linux kernels can return other errors.
          LOG_SYSERR << "unknown error of accept.";
          break;
      }
    } else if (connfd >= 0) {
      if (acceptor_cb_) {
        LOG_TRACE << "got a new connection.";
        acceptor_cb_(connfd);  // HTTPService::newConnection(int connfd)
      } else {
        utils::close(connfd);
      }
      return;
    }
  }
}

}  // namespace raver
