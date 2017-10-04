#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "../base/noncopyable.h"
#include "../base/Buffer.h"
#include "../base/FileCache.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPParser.h"
#include <functional>

namespace raver {

namespace detail {
    void handleHTTPCallback(const HTTPRequest&, HTTPResponse*);
}

class HTTPService;
class Channel;

using HTTPCallback = std::function<void (const HTTPRequest&, HTTPResponse*)>;

class HTTPConnection : noncopyable {
public:
    explicit HTTPConnection(HTTPService* service, int connfd);
    ~HTTPConnection();
    static FileCache* fileCache() { return &filecache_; }
private:
    void startRead();

    void doRead(); // read request.

    void doWrite(); // write response.

    bool parseRequestOK();

    bool handleRequest();

    void close();

private:
    //friend void detail::handleHTTPCallback(const HTTPRequest&, HTTPResponse*);
    HTTPService* service_; // not own it.

    int connfd_;
    bool done;
    Channel* channel_; // not own it.

    static FileCache filecache_;
    Buffer in_;
    Buffer out_;

    HTTPRequest request_;
    HTTPResponse response_;
    HTTPParser parser_;
    HTTPCallback cb_;
};

}


#endif
