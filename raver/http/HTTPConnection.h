#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <functional>
#include <memory>
#include "../base/Buffer.h"
#include "../base/noncopyable.h"
#include "HTTPParser.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

namespace raver {

class HTTPService;
class Channel;
class HTTPConnection;

using ConnectionCallback = std::function<void(const HTTPConnection&)>;
using CloseCallback = std::function<void(const HTTPConnection&)>;
using WriteCompleteCallback = std::function<void(const HTTPConnection&)>;
using MessageCallback = std::function<void(HTTPConnection&, Buffer*)>;

using HTTPCallback = std::function<void(const HTTPRequest&, HTTPResponse*)>;

class HTTPConnection : noncopyable {
 public:
  HTTPConnection(HTTPService* service, int connfd);

  ~HTTPConnection();

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  bool connected() const { return state_ == State::Connected; }

  const HTTPParser& getParser() const { return parser_; }

  void shutdown();

  void send(const char* data, size_t len);
  void send(const std::string& msg);
  void send(Buffer* buffer);

 private:
  enum class State { Disconnected, Connecting, Connected, Disconnecting };

  void setState(State state) { state_ = state; }

  void handleRead();   // read request.
  void handleWrite();  // write response.
  void handleClose();
  void handleError();

  bool parseRequestOK();

  bool handleRequest();

  void close();

 private:
  HTTPService* service_;  // not own it.

  State state_;

  int connfd_;
  bool done_;
  std::unique_ptr<Channel> channel_;

  Buffer input_buffer_;
  Buffer output_buffer_;

  HTTPRequest request_;
  HTTPResponse response_;
  HTTPParser parser_;

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  CloseCallback closeCallback_;
};

}  // namespace raver

#endif
