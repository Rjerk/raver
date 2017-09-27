#include "HTTPConnection.h"
#include "Channel.h"
#include "HTTPService.h"
#include "IOManager.h"
#include "../base/Utils.h"
#include "../base/Logger.h"
#include "../base/RJson.h"

#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace raver {

namespace {

constexpr size_t BUF_SIZE = 1024;

void defaultHTTPCallback(const HTTPRequest&, HTTPResponse* resp)
{
    resp->setStatusCode(HTTPResponse::HTTPStatusCode::NotFound404);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
    resp->setBody("<html>"
            "<head><title>404 Not Found</title></head>"
            "<body bgcolor=\"white\">"
            "<center><h1>404 Not Found</h1></center>"
            "<hr><center>raver</center>"
            "</body>"
            "</html>");
}

void handleHTTPCallback(const HTTPRequest& request, HTTPResponse* resp)
{
    LOG_INFO << "handleHTTPCallback";
    if (request.getMethod() != HTTPRequest::Method::Get
        && request.getMethod() != HTTPRequest::Method::Post) {
        LOG_INFO << "use default";
        defaultHTTPCallback(request, resp);
        return ;
    }

    bool post = false;
    if (request.getMethod() == HTTPRequest::Method::Post) {
        post = true;
    }

    rjson::RJSON parser(readFile("config.json"));
    parser.parseJson();
    auto doc_root = *(parser.getValue()->getValueFromObject("doc-root")->getString());
    LOG_INFO << "root: " << doc_root;
    LOG_INFO << "req:" << request.getPath();
    auto path = doc_root + request.getPath().substr(1);
    LOG_INFO << "path2: " << path;

    if (path.at(path.size()-1) == '/') {
        path += *(parser.getValue()->getValueFromObject("index-page")->getString());
    }
    LOG_INFO << "use path: " << path;

    struct stat st;
    if (::stat(path.c_str(), &st) == -1) {
        LOG_INFO << "use default!";
        defaultHTTPCallback(request, resp);
        return ;
    } else {
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            path += *(parser.getValue()->getValueFromObject("index-page")->getString());
        }
        if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) {
            post = true;
        }
    }
    LOG_INFO << "path:" << path;

    if (post) {
        //
    } else {
        resp->setStatusCode(HTTPResponse::HTTPStatusCode::OK200);
        resp->setStatusMessage("OK");
        resp->setCloseConnection(false);
        resp->setBody(readFile(path.c_str()));
    }
}

}

HTTPConnection::HTTPConnection(HTTPService* service, int connfd)
    : service_(service), connfd_(connfd), done(false), channel_(nullptr),
      in_(), out_(), request_(), response_(false), parser_(), cb_(handleHTTPCallback)
{
    LOG_INFO << "HTTPConnection ctor";
    channel_ = service_->ioManager()->newChannel(connfd, std::bind(&HTTPConnection::doRead, this),
                                                         std::bind(&HTTPConnection::doWrite, this));
    channel_->read();
}

HTTPConnection::~HTTPConnection()
{
    LOG_DEBUG << "HTTPConnection dtor" << this;
    if (service_) {
        service_->ioManager()->removeChannel(channel_);
    }
    close();
}

void HTTPConnection::close()
{
    if (connfd_ >= 0) {
        ::close(connfd_);
    }
}

void HTTPConnection::startRead()
{
    // we get a new connection, so socket can be read.
    channel_->read();
}

void HTTPConnection::doRead()
{
    int saved_errno;
    ssize_t n = in_.readFd(connfd_, &saved_errno);
    LOG_INFO << "--------size-------------: " << in_.size();

    if (n == 0) {
        LOG_INFO << "n == 0";
        return ;
    } else if (n < 0) {
        errno = saved_errno;
        LOG_SYSERR << "doRead: n < 0";
        return ;
    }

    if (!handleRequest()) {
        LOG_ERROR << "handle request failed.";
        return ;
    }

    LOG_DEBUG << "handle request ok.";
}

void HTTPConnection::doWrite()
{
    for ( ; ; ) {
        LOG_INFO << "out_size: ----------------- " << out_.size();
        ssize_t n = ::write(connfd_, out_.beginRead(), out_.readableBytes());
        if (n > 0) {
            LOG_INFO << "write: " << out_.beginRead();
            out_.retrieve(n);
            if (out_.readableBytes() == 0) {
                break;
            }
        } else if (n == -1 && errno == EAGAIN) {
            LOG_INFO << "get eagain.";
            break;
        } else if (n == -1) { // other error.
            LOG_ERROR << "write error." << strerror(errno);
            break;
        } else if (n == 0) {
            LOG_ERROR << "n == 0 in write.";
            break;
        }
    }
    // shutdown after send respond.
    LOG_INFO << "we close the connection after send response.";
    if (response_.closeConnectionOrNot()) {
        close();
    }
}

bool HTTPConnection::parseRequestOK()
{
    request_.clear();
    bool ret = parser_.parseRequest(&in_, &request_);
    if (!ret) { // send 404 bad request.
        LOG_ERROR << "Parse HTTP request error.";
        return false;
    }
    LOG_DEBUG << "parse request ok.";
    return true;
}

bool HTTPConnection::handleRequest()
{
    if (!parseRequestOK()) {
        LOG_ERROR << "parse request failed.";
        return false;
    }
    if (parser_.gotAll()) {
        const std::string& connection = request_.getHeader("Connection");
        LOG_INFO << "connection: " << connection;
        bool close_req = (connection == "close")
                || (request_.getVersion() == HTTPRequest::Version::HTTP10
                    && connection != "keep-alive");
        cb_(request_, &response_);
        response_.setCloseConnection(close_req);
        response_.appendToBuffer(&out_); // write respond to output buffer.
        return true;
    } else {
        return false;
    }
}


}
