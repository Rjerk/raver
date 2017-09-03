#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include "noncopyable.h"

#include <functional>
#include <string>

namespace raver {

class ServiceManager;
class IOManager;
class HTTPClientConnection;
class HTTPRespond;

using ConnectCallback = std::function<void (HTTPClientConnection*)>;
using RespondCallback = std::function<void (HTTPRespond* )>;

class HTTPService : noncopyable {
public:
    HTTPService(int port, ServiceManager* sm); // serve in a port and cooperate with the manager.

    ~HTTPService();

    void stop(); // tell manager I'm done.

    //void asyncConnect(const std::string& host, int port, const ConnectCallback& cb);

    //void connect(const std::string& host, int port, HTTPClientConnection** conn);

    IOManager* ioManager() const;
private:
    void afterAccept(int port);

private:
    ServiceManager* manager_;

};

}

#endif
