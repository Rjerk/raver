#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include "../base/noncopyable.h"

#include <functional>
#include <string>
#include <vector>

namespace raver {

class ServiceManager;
class IOManager;
class HTTPRespond;
class HTTPConnection;

using RespondCallback = std::function<void (HTTPRespond* )>;

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
};

}

#endif
