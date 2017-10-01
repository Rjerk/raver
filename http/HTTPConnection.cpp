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

void send404(HTTPResponse* resp)
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

void send501(HTTPResponse* resp)
{
    resp->setStatusCode(HTTPResponse::HTTPStatusCode::NotImp501);
    resp->setStatusMessage("Method Not Implemented");
    resp->setContentType("text/html");
    resp->setBody("<html>"
            "<head><title>505 Not Implemented</title></head>"
            "<body bgcolor=\"white\">"
            "<center><h1>501 Not Implemented</h1></center>"
            "<hr><center>raver</center>"
            "</body>"
            "</html>");
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

    rjson::RJSON parser(readFile("config.json"));
    parser.parseJson();
    auto doc_root = *(parser.getValue()->getValueFromObject("doc-root")->getString());
    LOG_TRACE << "root: " << doc_root;
    LOG_TRACE << "req:" << request.getPath();
    auto path = doc_root + request.getPath().substr(1);
    LOG_TRACE << "path: " << path;

    if (path.at(path.size()-1) == '/') {
        path += *(parser.getValue()->getValueFromObject("index-page")->getString());
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
            path += *(parser.getValue()->getValueFromObject("index-page")->getString());
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
        LOG_TRACE << "use GET";
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
    LOG_TRACE << "HTTPConnection ctor";
    channel_ = service_->ioManager()->newChannel(connfd, std::bind(&HTTPConnection::doRead, this),
                                                         std::bind(&HTTPConnection::doWrite, this));
    channel_->read();
}

HTTPConnection::~HTTPConnection()
{
    LOG_TRACE << "HTTPConnection dtor" << this;
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
    LOG_TRACE << "in_ size: " << in_.size();

    if (n == 0) {
        LOG_DEBUG << "in_.readFd: n == 0";
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

    LOG_TRACE << "handle request ok.";
}

void HTTPConnection::doWrite()
{
    for ( ; ; ) {
        LOG_TRACE << "out_ size: " << out_.size();
        ssize_t n = ::write(connfd_, out_.beginRead(), out_.readableBytes());
        if (n > 0) {
            LOG_TRACE << "write " << n << " bytes to connfd_";
            out_.retrieve(n);
            if (out_.readableBytes() == 0) {
                break;
            }
        } else if (n == -1 && errno == EAGAIN) {
            LOG_SYSERR << "get eagain.";
            break;
        } else if (n == -1) { // other error.
            LOG_SYSERR << "write error." << strerror(errno);
            break;
        } else if (n == 0) {
            LOG_ERROR << "write: n == 0.";
            break;
        }
    }

    // TODO: shutdown after send respond.
    if (response_.closeConnectionOrNot()) {
        LOG_TRACE << "we close the connection after send response.";
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
    LOG_TRACE << "parse request ok.";
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
        LOG_TRACE << "connection: " << connection;
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
