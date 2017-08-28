#ifndef HTTP_CLIENT_CONNECTION_H
#define HTTP_CLIENT_CONNECTION_H

#include <functional>

namespace raver {

class HTTPRequest;
class HTTPRespond;
class HTTPService;
class HTTPClientConnection;

using RespondCallback = std::function<void (HTTPRespond* )>;
using ConnectCallback = std::function<void (HTTPClientConnection*)>;

class HTTPClientConnection {
public:
    explicit HTTPClientConnection(HTTPService* service);

    void connect(const std::string& host, int port, const ConnectCallback& cb);

    void asyncSend(HTTPRequest* request, RespondCallback cb);

    void send(HTTPRequest* request, HTTPRespond** respond);

private:

private:
    HTTPService* service_;

};

}

#endif
