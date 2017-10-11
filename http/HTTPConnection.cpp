#include "HTTPConnection.h"
#include "Channel.h"
#include "HTTPService.h"
#include "IOManager.h"
#include "../base/Utils.h"
#include "../base/Logger.h"
#include "../base/RJson.h"
#include "../base/FileCache.h"

#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace raver {

FileCache HTTPConnection::filecache_{50 << 20};

namespace detail {

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
        Buffer* buf = nullptr;
        HTTPConnection::fileCache()->pin(path.c_str(), &buf);
        LOG_TRACE << "use GET";
        resp->setStatusCode(HTTPResponse::HTTPStatusCode::OK200);
        resp->setStatusMessage("OK");
        resp->setCloseConnection(true);
        resp->setBody(std::string(buf->beginRead(), buf->size()));
    }
}

}

HTTPConnection::HTTPConnection(HTTPService* service, int connfd)
    : service_(service), connfd_(connfd), done(false), channel_(nullptr),
      in_(), out_(), request_(), response_(false), parser_(), cb_(detail::handleHTTPCallback)
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
    LOG_TRACE << "doRead begin.";
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

    in_.clear();


    LOG_TRACE << "doRead end";
}

void HTTPConnection::doWrite()
{
    LOG_TRACE << "doWrite begin " << this;

    int writen_bytes = 0;
    while ((writen_bytes = ::write(connfd_, out_.beginRead(), out_.readableBytes())) > 0) {
        LOG_TRACE << "write connfd_: " << writen_bytes;
        LOG_TRACE << "readableBytes: " << out_.readableBytes();
        assert(static_cast<size_t>(writen_bytes) <= out_.readableBytes());
        out_.retrieve(writen_bytes);
    }

    if (out_.readableBytes() == 0 && response_.closeConnectionOrNot()) {
        LOG_TRACE << "we close the connection after send response.";
        shutdown(connfd_, SHUT_WR);
    }

    if (writen_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        LOG_SYSERR << "EAGAIN write error";

        return ;
    }

    if (writen_bytes < 0) {
        LOG_SYSERR << "write error";
        return ;
    }

    out_.clear();

    service_->ioManager()->poller()->updateEvent(EPOLLOUT, EPOLL_CTL_DEL, channel_);

    LOG_TRACE << "doWrite end";
}

bool HTTPConnection::parseRequestOK()
{
    LOG_TRACE << "";
    request_.clear();
    in_.append("\0", 1);
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
    LOG_TRACE << "";

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
