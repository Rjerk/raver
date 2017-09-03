#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "noncopyable.h"
#include "Buffer.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

#include <functional>

namespace raver {

class HTTPService;
class Channel;

using HTTPCallback = std::function<void (const HTTPRequest&, HTTPResponse*)>;

class HTTPConnection : noncopyable {
public:
    explicit HTTPConnection(HTTPService* service, int connfd);
    ~HTTPConnection();
private:
    void startRead();

    void doRead(); // read request.

    void doWrite(); // write respond.

    bool parseRequestOK();

    bool handleRequest();
private:
    HTTPService* service_;
    int connfd_;
    Channel* channel_;

    Buffer in_;
    Buffer out_;

    HTTPRequest request_;
    HTTPResponse response_;
};


}


#endif
