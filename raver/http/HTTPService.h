#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <functional>
#include <vector>
#include <raver/base/FileCache.h>
#include <raver/base/noncopyable.h>

namespace raver {

class ServiceManager;
class HTTPConnection;
class Buffer;
class HTTPRequest;
class HTTPResponse;

class HTTPService : noncopyable {
 public:
  explicit HTTPService(ServiceManager* manager);

  ~HTTPService();

  void newConnection(int connfd);

  ServiceManager* serviceManager() const { return service_manager_; }

  static FileCache* fileCache() { return &filecache_; }

 private:
  void onConnection(const HTTPConnection& conn);

  void onMessage(HTTPConnection& conn, Buffer* buffer);

 private:
  ServiceManager* service_manager_;  // not own it.

  std::vector<HTTPConnection*> conns_;

  using HTTPCallback = std::function<void(const HTTPRequest&, HTTPResponse*)>;
  HTTPCallback httpCallback_;

  static FileCache filecache_;
};

}  // namespace raver

#endif
