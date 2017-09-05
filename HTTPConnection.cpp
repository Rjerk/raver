#include "HTTPConnection.h"
#include "Channel.h"
#include "HTTPService.h"
#include "IOManager.h"
#include "Utils.h"
#include "Logger.h"

#include <unistd.h>
#include <strings.h>
#include <functional>

namespace raver {

namespace {

void defaultHTTPCallback(const HTTPRequest&, HTTPResponse* resp)
{
    resp->setStatusCode(HTTPResponse::HTTPStatusCode::NotFound404);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

constexpr size_t BUF_SIZE = 1024;

}

HTTPConnection::HTTPConnection(HTTPService* service, int connfd)
    : service_(service), connfd_(connfd), channel_(nullptr),
      in_(), out_(), request_(), response_(false), parser_(), cb_(defaultHTTPCallback)
{
    LOG_INFO << "HTTPConnection ctor";
    channel_ = service_->ioManager()->newChannel(connfd, std::bind(&HTTPConnection::doRead, this),
                                                         std::bind(&HTTPConnection::doWrite, this));
    startRead();
}

HTTPConnection::~HTTPConnection()
{
    LOG_DEBUG << "HTTPConnection dtor";
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
    LOG_DEBUG << "startRead begin";
    channel_->readWhenReady();
    LOG_DEBUG << "startRead end";
}

void HTTPConnection::doRead()
{
    LOG_DEBUG << "doRead begin";

    for ( ; ; ) {
        int n = ::read(connfd_, in_.beginWrite(), in_.writableBytes());
        if (n > 0) {
            LOG_INFO << "read : " << in_.beginWrite();
            in_.hasWriten(n);
        }

        if (n < 0 && errno == EAGAIN) {
            channel_->readWhenReady();
            break;
        } else if (n < 0) {
            LOG_ERROR << "n < 0 in read.";
            break;
        } else if (n == 0) {
            LOG_WARN << "n == 0 in read.";
            break;
        } else if (!parseRequestOK()) {
            LOG_ERROR << "parse request failed.";
            break;
        }
    }
    LOG_DEBUG << "doRead end";
}

void HTTPConnection::doWrite()
{
    LOG_DEBUG << "doWrite begin.";
    for ( ; ; ) {
        ssize_t n = ::write(connfd_, out_.beginRead(), out_.readableBytes());
        if (n > 0) {
            LOG_INFO << "write: " << out_.beginRead();
            out_.hasWriten(n);
            if (out_.readableBytes() == 0) {
                break;
            }
        } else if (n == -1 && errno == EAGAIN) {
            channel_->writeWhenReady();
            break;
        } else if (n == -1) {
            LOG_ERROR << "write error." << strerror(errno);
            break;
        } else if (n == 0) {
            LOG_ERROR << "n == 0 in write.";
            break;
        }

    }
    // shutdown after send respond.
    close();
    LOG_DEBUG << "doWrite() end";
}

bool HTTPConnection::parseRequestOK()
{
    LOG_DEBUG << "parseRequestOK begin";
    request_.clear();
    for ( ; ; ) {
        bool ret = parser_.parseRequest(&in_, &request_);

        if (!ret) { // send 404 bad request.
            LOG_ERROR << "Parse HTTP request error.";
            return false;
        }

        if (!handleRequest()) {
    LOG_DEBUG << "parseRequestOK end1";
            return false;
        } else {
    LOG_DEBUG << "parseRequestOK end2";
            return true;
        }
    }
}

bool HTTPConnection::handleRequest()
{
    if (parser_.gotAll()) {
        const std::string& connection = request_.getHeader("Connection");
        LOG_DEBUG << "connection: " << connection;
        bool close_req = (connection == "close")
                || (request_.getVersion() == HTTPRequest::Version::HTTP10
                    && connection != "keep-alive");
        cb_(request_, &response_);
        response_.setCloseConnection(close_req);
        response_.appendToBuffer(&out_); // write respond to output buffer.
        LOG_DEBUG << "buffer: " << out_.beginRead();
        return true;
    } else {
        return false;
    }
}

}
