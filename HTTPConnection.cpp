#include "HTTPConnection.h"
#include "Channel.h"
#include "HTTPService.h"
#include "IOManager.h"
#include "Utils.h"
#include "Logger.h"


#include <functional>

namespace raver {

HTTPConnection::HTTPConnection(HTTPService* service, int connfd)
    : service_(service), connfd_(connfd)
{
    channel_ = service_->ioManager()->newChannel(connfd, std::bind(&HTTPConnection::doRead, this),
                                                         std::bind(&HTTPConnection::doWrite, this));
    startRead();
}

void HTTPConnection::startRead()
{
    channel_->readWhenReady();
}

void HTTPConnection::doRead()
{
    char buf[1024];
    for ( ; ; ) {
        int n = wrapper::read(connfd_, buf, sizeof(buf));
        if (n == 0) {
            break; // connfd is closed.
       }
        LOG_INFO << "read buf: " << buf;
    }
}

void HTTPConnection::doWrite()
{
    char buf[1024];
    // ???

    for ( ; ; ) {
        int n = wrapper::write(connfd_, buf, sizeof(buf));
        if (n == 0) {
            break; // connfd is closed.
        }
    }
}

}
