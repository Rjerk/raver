#include "HTTPConnection.h"
#include "Channel.h"
#include "HTTPService.h"
#include "IOManager.h"
#include "Utils.h"
#include "Logger.h"
#include "HTTPParser.h"

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
      in_(), out_(), request_(), response_()
{
    LOG_INFO << "HTTPConnection ctor";
    channel_ = service_->ioManager()->newChannel(connfd, std::bind(&HTTPConnection::doRead, this),
                                                         std::bind(&HTTPConnection::doWrite, this));
    startRead();
}

HTTPConnection::~HTTPConnection()
{
    LOG_DEBUG << "HTTPConnection dtor";
    service_->ioManager()->removeChannel(channel_);
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
}

void HTTPConnection::doWrite()
{
    // do nothing.
}

bool HTTPConnection::parseRequestOK()
{
    for ( ; ; ) {
        request_.clear();
        bool ret = HTTPParser::parseRequest(&in_, &request_);

        if (!ret) {
            LOG_ERROR << "Parse HTTP request error.";
            return false;
        }

        if (!handleRequest()) {
            return false;
        } else {
            return true;
        }
    }
}

bool HTTPConnection::handleRequest()
{
    if (HTTPParser::gotAll()) {
        // TODO: respond;
    }
}

}
