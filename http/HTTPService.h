#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include "../base/noncopyable.h"
#include "../base/FileCache.h"

#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include <memory>

namespace raver {

class ServiceManager;
class IOManager;
class HTTPRespond;
class HTTPConnection;

using RespondCallback = std::function<void (HTTPRespond* )>;
using MutexGuard = std::lock_guard<std::mutex>;

class HTTPService : noncopyable {
public:
    HTTPService(int port, ServiceManager* sm); // serve in a port and cooperate with the manager.

    ~HTTPService();

    void stop(); // tell manager I'm done.

    IOManager* ioManager() const;
private:
    void afterAccept(int port);

private:
    ServiceManager* manager_; // not own it.

    std::vector<HTTPConnection*> connections_;
    std::mutex mtx_vec_;
};

}

#endif
