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

void HTTPService::afterAccept(int connfd)
{
    if (manager_->isStopped()) {
        LOG_DEBUG << "manager_ stopped.";
        return ;
    }
    if (connfd < 0) {
        LOG_ERROR << "Client fd:" << connfd;
        stop();
        return ;
    }

    HTTPConnection* conn = new HTTPConnection(this, connfd); (void) conn;
}

IOManager* HTTPService::ioManager() const
{
    return manager_->ioManager();
}

}


