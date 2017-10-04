#include "HTTPService.h"
#include "ServiceManager.h"
#include "HTTPConnection.h"
#include "../base/Logger.h"
#include "../base/FileCache.h"

namespace raver {

HTTPService::HTTPService(int port, ServiceManager* sm)
    : manager_(sm)
{
    manager_->registerAcceptor(port, std::bind(&HTTPService::afterAccept, this, std::placeholders::_1));
    LOG_TRACE << "HTTPService ctor";
}

HTTPService::~HTTPService()
{
    LOG_TRACE << "HTTPService dtor";
    for (auto conn : connections_) {
        delete conn;
    }
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

    HTTPConnection* conn = new HTTPConnection(this, connfd);
    {
        MutexGuard guard(mtx_vec_);
        connections_.push_back(conn);
    }
}

IOManager* HTTPService::ioManager() const
{
    return manager_->ioManager();
}

}


