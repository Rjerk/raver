#include "HTTPConnection.h"
#include "../base/RJson.h"
#include "../base/Utils.h"
#include "Channel.h"
#include "HTTPService.h"
#include "IOManager.h"
#include "ServiceManager.h"

#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

namespace raver {

HTTPConnection::HTTPConnection(HTTPService* service, int connfd)
    : service_(service),
      state_(State::Connecting),
      connfd_(connfd),
      done_(false),
      channel_(new Channel(service_->serviceManager()->ioManager(), connfd)),
      input_buffer_(),
      output_buffer_(),
      request_(),
      response_(false),
      parser_() {
  LOG_TRACE << "HTTPConnection ctor. fd = " << connfd_;
  channel_->setReadCallback(std::bind(&HTTPConnection::handleRead, this));
  channel_->setWriteCallback(std::bind(&HTTPConnection::handleWrite, this));
  channel_->setErrorCallback(std::bind(&HTTPConnection::handleError, this));
  channel_->setErrorCallback(std::bind(&HTTPConnection::handleClose, this));
  channel_->enableReading();

  const int opt = 1;
  ::setsockopt(connfd_, SOL_SOCKET, SO_KEEPALIVE, &opt,
               static_cast<socklen_t>(sizeof(opt)));
}

HTTPConnection::~HTTPConnection() {
  LOG_TRACE << "HTTPConnection dtor" << this;
  assert(state_ == State::Disconnected);
  wrapper::close(connfd_);
}

void HTTPConnection::handleRead() {
  LOG_TRACE << "doRead begin.";
  setState(State::Connected);

  int saved_errno = 0;
  ssize_t n = input_buffer_.readFd(connfd_, &saved_errno);
  LOG_TRACE << "in_ size: " << input_buffer_.size();

  if (n > 0) {
    LOG_TRACE << "doRead: n > 0, message_callback()";
    messageCallback_(*this, &input_buffer_);
  } else if (n == 0) {
    LOG_TRACE << "doRead: n == 0, handleClose()";
    handleClose();
  } else {
    errno = saved_errno;
    LOG_SYSERR << "doRead: n < 0";
    // handleError();
  }
}

void HTTPConnection::handleWrite() {
  LOG_TRACE << "doWrite begin " << this;

  if (channel_->isWriting()) {
    ssize_t n;
    if ((n = wrapper::write(channel_->fd(), output_buffer_.beginRead(),
                            output_buffer_.readableBytes())) > 0) {
      output_buffer_.retrieve(n);
      if (output_buffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          writeCompleteCallback_(*this);
        }

        if (state_ == State::Disconnecting) {
          shutdown();
        }
      }
    }

  } else {
    LOG_TRACE << "Connection fd: " << channel_->fd()
              << " is down, no more writing.";
  }

  LOG_TRACE << "doWrite end";
}

void HTTPConnection::handleError() { LOG_ERROR << "handleError"; }

void HTTPConnection::handleClose() {
  LOG_TRACE << "fd = " << channel_->fd();
  assert(state_ == State::Connected || state_ == State::Disconnecting);

  setState(State::Disconnected);
  channel_->disableAll();

  // callback
}

void HTTPConnection::send(const char* data, size_t len) {
  LOG_TRACE << "sending data...";
  if (state_ == State::Disconnected) {
    LOG_ERROR << "Disconnected, give up writing.";
    return;
  }

  LOG_TRACE << "start write the data, len: " << len;

  bool fault_error = false;
  ssize_t nwrote = 0;
  size_t remaining = len;
  if (!channel_->isWriting() && output_buffer_.readableBytes() == 0) {
    nwrote = wrapper::write(channel_->fd(), data, len);
    LOG_TRACE << "nwrote: " << nwrote;
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_) {
        LOG_TRACE << "write complete.";
        writeCompleteCallback_(*this);
        return;
      }
    } else {
      nwrote = 0;
      if (errno != EAGAIN) {
        LOG_SYSERR << "write error.";
        if (errno == EPIPE || errno == ECONNRESET) {
          fault_error = true;
        }
      }
    }
  }

  LOG_TRACE << "enableWriting";

  assert(remaining <= len);
  if (!fault_error && remaining > 0) {
    output_buffer_.append(data + nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

void HTTPConnection::send(Buffer* buf) {
  assert(state_ == State::Connected);
  if (state_ == State::Connected) {
    send(buf->peek(), buf->readableBytes());
    buf->retrieveAll();
  }
}

void HTTPConnection::send(const std::string& msg) {
  if (state_ == State::Connected) {
    send(msg.data(), msg.size());
  }
}

void HTTPConnection::shutdown() {
  LOG_TRACE << "shutdowning the connection...";
  if (state_ == State::Connected) {
    setState(State::Disconnecting);

    if (!channel_->isWriting()) {
      if (::shutdown(connfd_, SHUT_WR) < 0) {
        LOG_SYSERR << "shutdown error.";
      }
    }
  }
}

}  // namespace raver
