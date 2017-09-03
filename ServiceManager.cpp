#include "ServiceManager.h"
#include "IOManager.h"
#include "Logger.h"

namespace raver {

ServiceManager::ServiceManager(int num_thread)
    : num_thread_(num_thread), stop_requested_(false), stopped_(false),
      io_(new IOManager(num_thread_))
{
    LOG_DEBUG << "ServiceManager ctor";
}

ServiceManager::~ServiceManager()
{
    LOG_DEBUG << "ServiceManager dtor";
    stop();
    for (auto ac : acceptors_) {
        delete ac;
    }
    delete io_;
}

void ServiceManager::run()
{
    LOG_DEBUG << "run begin";
    for (auto ac : acceptors_) {
        LOG_DEBUG << "ac->startAccept()";
        ac->startAccept();
    }

    LOG_INFO << "server polling in.";
    io_->poll();
    LOG_INFO << "server polling out";

    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&](){ return stopped_ == true; });
    }

    LOG_DEBUG << "run end";
}

void ServiceManager::stop()
{
    LOG_DEBUG << "stop begin";
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (stop_requested_) {
            stopped_ = true;
        }
    }

    for (auto ac : acceptors_) {
        ac->close();
    }

    io_->stop();

    cv_.notify_all();

    LOG_DEBUG << "stop end";
}

void ServiceManager::registerAcceptor(int port, AcceptorCallback cb)
{
    LOG_DEBUG << "registerAcceptor begin";
    acceptors_.push_back(new Acceptor(io_, port, cb));
    LOG_DEBUG << "registerAcceptor end";
}

}
