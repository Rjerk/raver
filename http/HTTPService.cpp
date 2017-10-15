#include "../base/Logger.h"
#include "../base/RJson.h"
#include "HTTPService.h"
#include "ServiceManager.h"
#include "IOManager.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "Acceptor.h"
#include "HTTPConnection.h"

#include <unistd.h>
#include <sys/stat.h>

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

void handleHTTPCallback(const HTTPRequest& request, HTTPResponse* resp)
{
    LOG_TRACE << "handleHTTPCallback";
    if (request.getMethod() != HTTPRequest::Method::Get
                    && request.getMethod() != HTTPRequest::Method::Post) {
            LOG_TRACE << "neither get nor post method";
            send501(resp);
            return ;
        }

    bool post = false;
    if (request.getMethod() == HTTPRequest::Method::Post) {
            post = true;
        }

    /*
    rjson::RJSON parser(readFile("config.json"));
    parser.parseJson();
    auto doc_root = *(parser.getValue()->getValueFromObject("doc-root")->getString());
    */
    std::string doc_root = "./www/";
    LOG_TRACE << "root: " << doc_root;
    LOG_TRACE << "req: " << request.getPath();
    auto path = doc_root + request.getPath().substr(1);
    LOG_TRACE << "path: " << path;

    if (path.at(path.size()-1) == '/') {
        //path += *(parser.getValue()->getValueFromObject("index-page")->getString());
        path += "index.html";
    }
    LOG_TRACE << "use path: " << path;

    struct stat st;
    bool can_exe = false;
    if (::stat(path.c_str(), &st) == -1) {
        LOG_TRACE << "find file failed. use default";
        send404(resp);
        return ;
    } else {
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            LOG_TRACE << "path is a directory";
            //path += *(parser.getValue()->getValueFromObject("index-page")->getString());
            path += "index.html";
        }
        if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) {
            LOG_TRACE << "file is executable";
            can_exe = true;
        }
    }
    if (post) {
        LOG_TRACE << "use POST";
        // TODO: cgi program.
        LOG_TRACE << "execute cgi-program";
        if (can_exe) {
            ::execl(path.c_str(), path.c_str(), nullptr);
        }
    } else {
        //Buffer* buf = nullptr;
        //HTTPService::fileCache()->pin(path.c_str(), &buf);
        LOG_TRACE << "use GET";
        resp->setStatusCode(HTTPResponse::HTTPStatusCode::OK200);
        resp->setStatusMessage("OK");
        resp->setCloseConnection(true);
        //resp->setBody(std::string(buf->beginRead(), buf->size()));
        resp->setBody(readFile(path.c_str()));
    }
}

} // namespace detail

FileCache HTTPService::filecache_{50 << 20};

HTTPService::HTTPService(ServiceManager* manager, int port)
    : service_manager_(manager),
      conns_(),
      httpCallback_(detail::handleHTTPCallback)
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

void HTTPService::onMessage(HTTPConnection& conn, Buffer* buffer)
{
    LOG_TRACE << "start handle the request.";
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
