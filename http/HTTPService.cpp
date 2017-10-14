#include "../base/Logger.h"
#include "HTTPService.h"
#include "ServiceManager.h"
#include "IOManager.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "Acceptor.h"
#include "HTTPConnection.h"

namespace raver {

namespace detail {

#define msg(status_and_msg) ( \
    "<html><head><title>" \
    status_and_msg \
    "</title></head><body bgcolor=\"white\"><center><h1>" \
    status_and_msg \
    "</h1></center><hr><center>raver</center></body></html>" \
)

void send404(HTTPResponse* resp)
{
    resp->setStatusCode(HTTPResponse::HTTPStatusCode::NotFound404);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
    resp->setBody(msg("404 Not Found"));
}

void send501(HTTPResponse* resp)
{
    resp->setStatusCode(HTTPResponse::HTTPStatusCode::NotImp501);
    resp->setStatusMessage("Method Not Implemented");
    resp->setContentType("text/html");
    resp->setBody(msg("501 Not Implemented"));
}

};

HTTPService::HTTPService(ServiceManager* manager, int port)
    : service_manager_(manager)
{
    service_manager_->registerAcceptor(port, std::bind(&HTTPService::newConnection, this, std::placeholders::_1));
    LOG_TRACE << "HTTPService ctor";
}

HTTPService::~HTTPService()
{
    for (auto& c : conns_) {
        delete c;
    }
}

void HTTPService::newConnection(int connfd)
{
    auto conn = new HTTPConnection(this, connfd);
    using std::placeholders::_1;
    using std::placeholders::_2;
    conn->setConnectionCallback(std::bind(&HTTPService::onConnection, this, _1));
    conn->setMessageCallback(std::bind(&HTTPService::onMessage, this, _1, _2));
    conns_.push_back(conn);
}

void HTTPService::onConnection(const HTTPConnection&)
{
    LOG_TRACE << "got a request.";
}

void HTTPService::onMessage(const HTTPConnection& conn, Buffer* buffer)
{
    auto parser = conn.getParser();

    if (!parser.parseRequest(buffer)) {
        conn.send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn.shutdown();
    }

    if (parser.gotAll()) {
        auto request = parser.request();
        const std::string& connection = request.getHeader("Connection");
        LOG_TRACE << "connection: " << connection;
        bool close_req = (connection == "close")
                    || (request.getVersion() == HTTPRequest::Version::HTTP10
                                        && connection != "keep-alive");

        HTTPResponse response(close_req);

        service_manager_->ioManager()->addTask([&](){ httpCallback_(request, &response); });

        Buffer out_buf;
        response.appendToBuffer(&out_buf); // write respond to output buffer.

        conn.send(&out_buf);

        if (response.closeConnection()) {
            conn.shutdown();
        }
    }
}

}
