#ifndef HTTP_SERVER_CONNECTION
#define HTTP_SERVER_CONNECTION

namespace raver {

class HTTPService;

class HTTPServerConnection {
public:
    explicit HTTPServerConnection(HTTPService* service, int clientfd);

private:
    HTTPService* service_;


};


}


#endif
