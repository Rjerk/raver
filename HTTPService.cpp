#include "HTTPService.h"
#include "ServiceManager.h"
#include "Logger.h"
#include "HTTPConnection.h"
#include "Notification.h"

namespace raver {

HTTPService::HTTPService(int port, ServiceManager* sm)
    : manager_(sm)
{
    manager_->registerAcceptor(port, std::bind(&HTTPService::afterAccept, this, std::placeholders::_1));
    LOG_INFO << "HTTPService ctor";
}

HTTPService::~HTTPService()
{
    LOG_DEBUG << "HTTPService dtor";
}

void HTTPService::stop()
{
    manager_->stop();
}

/*
void HTTPService::asyncConnect(const std::string& host, int port, const ConnectCallback& cb)
{
    if (manager_->isStopped()) {
        return ;
    }

    //HTTPClientConnection conn(this);
    //conn.connect(host, port, cb);

}

void HTTPService::connect(const std::string& host, int port, HTTPClientConnection** conns)
{
    Notification n;
    asyncConnect(host, port, [&](HTTPClientConnection* new_conn)
                             {
                                *conns = new_conn;
                                n.notify();
                             });
    n.wait();
}
*/

void HTTPService::afterAccept(int connfd)
{
    LOG_DEBUG << "service accept begin";
    if (manager_->isStopped()) {
        return ;
    }
    if (connfd < 0) {
        LOG_ERROR << "Client fd:" << connfd;
        stop();
        return ;
    }

    HTTPConnection* conn = new HTTPConnection(this, connfd); (void) conn;
    LOG_DEBUG << "service accept end";
}

IOManager* HTTPService::ioManager() const
{
    return manager_->ioManager();
}

}


