#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "../base/noncopyable.h"
#include "../base/Buffer.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPParser.h"

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

    void doWrite(); // write response.

    bool parseRequestOK();

    bool handleRequest();

    void close();
private:
    HTTPService* service_; // not own it.
    int connfd_;
    bool done;
    Channel* channel_; // not own it.

    Buffer in_;
    Buffer out_;

    HTTPRequest request_;
    HTTPResponse response_;
    HTTPParser parser_;
    HTTPCallback cb_;
};


}


#endif
