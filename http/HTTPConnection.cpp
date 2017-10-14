#include "HTTPConnection.h"
#include "Channel.h"
#include "HTTPService.h"
#include "ServiceManager.h"
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
    : service_(service),
      state_(kConnecting),
      connfd_(connfd),
      done_(false),
      channel_(new Channel(service_->serviceManager()->ioManager(), connfd)),
      input_buffer_(),
      output_buffer_(),
      request_(),
      response_(false),
      parser_()
{
    LOG_TRACE << "HTTPConnection ctor. fd = " << connfd_;
    channel_->setReadCallback(std::bind(&HTTPConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&HTTPConnection::handleWrite, this));
    channel_->setErrorCallback(std::bind(&HTTPConnection::handleError, this));
    channel_->setErrorCallback(std::bind(&HTTPConnection::handleClose, this));

    const int opt = 1;
    ::setsockopt(connfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));
}

HTTPConnection::~HTTPConnection()
{
    LOG_TRACE << "HTTPConnection dtor" << this;
    assert(state_ == kDisconnected);
    wrapper::close(connfd_);
}

void HTTPConnection::handleRead()
{
    LOG_TRACE << "doRead begin.";
    int saved_errno = 0;
    ssize_t n = input_buffer_.readFd(connfd_, &saved_errno);
    LOG_TRACE << "in_ size: " << input_buffer_.size();

    if (n > 0) {
        LOG_TRACE << "doRead: n > 0, message_callback()";
        messageCallback_(*this, &input_buffer_);
    } else if (n == 0) {
        LOG_TRACE << "doRead: n == 0, handleClose()";
        handleClose();
    } else {
        errno = saved_errno;
        LOG_SYSERR << "doRead: n < 0";
        handleError();
    }

    if (!handleRequest()) {
        LOG_ERROR << "handle request failed.";
        return ;
    }

}

void HTTPConnection::handleWrite()
{
    LOG_TRACE << "doWrite begin " << this;

    if (channel_->isWriting()) {
        ssize_t n;
        if ((n = wrapper::write(channel_->fd(), output_buffer_.beginRead(), output_buffer_.readableBytes())) > 0) {
            output_buffer_.retrieve(n);
            if (output_buffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    writeCompleteCallback_(*this);
                }

                if (state_ == kDisconnecting) {
                    shutdown();
                }
            }
        }

    } else {
        LOG_TRACE << "Connection fd: " << channel_->fd()
                  << " is down, no more writing.";
    }

    /*
    int writen_bytes = 0;
    while ((writen_bytes = wrapper::write(connfd_, out_.beginRead(), out_.readableBytes())) > 0) {
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
    */

    LOG_TRACE << "doWrite end";
}

void HTTPConnection::handleError()
{
    LOG_ERROR << "handleError";
}

void HTTPConnection::handleClose()
{
    LOG_TRACE << "fd = " << channel_->fd();
    assert(state_ == kConnected || state_ == kDisconnecting);

    setState(kDisconnected);
    channel_->disableAll();

    // callback
}

bool HTTPConnection::parseRequestOK()
{
    LOG_TRACE << "";
    request_.clear();
    input_buffer_.append("\0", 1);
    bool ret = parser_.parseRequest(&input_buffer_, &request_);
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
        response_.appendToBuffer(&output_buffer_); // write respond to output buffer.
        return true;
    } else {
        return false;
    }
}


}
