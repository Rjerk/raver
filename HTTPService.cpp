#include "HTTPService.h"
#include "ServiceManager.h"
#include "Logger.h"
#include "HTTPConnection.h"
#include "Notification.h"

namespace raver {

HTTPService::HTTPService(int port, ServiceManager* sm)
    : manager_(sm)
{
    manager_->registry(port, std::bind(&HTTPService::accept, this, std::placeholders::_1));
    LOG_INFO << "HTTPService ctor";
}

HTTPService::~HTTPService()
{
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

void HTTPService::accept(int connfd)
{
    if (manager_->isStopped()) {
        return ;
    }
    if (connfd < 0) {
        LOG_ERROR << "Client fd:" << connfd;
        stop();
        return ;
    }

    HTTPConnection conn(this, connfd); // build a connection and start read.
}

IOManager* HTTPService::ioManager() const
{
    return manager_->ioManager();
}

}


