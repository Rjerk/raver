#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

namespace raver {

class HTTPService;
class Channel;

class HTTPConnection {
public:
    explicit HTTPConnection(HTTPService* service, int connfd);

private:
    void startRead();
    void doRead(); // read request.
    void doWrite(); // write respond.
private:
    HTTPService* service_;
    int connfd_;
    Channel* channel_;
};


}


#endif
