#include "ServiceManager.h"
#include "IOManager.h"

namespace raver {

ServiceManager::ServiceManager(int num_thread)
    : num_thread_(num_thread), stop_requested_(false), stopped_(false)
{
    io_ = new IOManager(num_thread_);
}

ServiceManager::~ServiceManager()
{
    for (auto ac : acceptors_) {
        delete ac;
    }
    delete io_;
}

void ServiceManager::run()
{
    for (auto ac : acceptors_) {
        ac->accept();
    }

    io_->poll();

    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&](){ return stopped_ == true; });
    }
}

void ServiceManager::stop()
{
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
}

void ServiceManager::registry(int port, AcceptorCallback cb)
{
    acceptors_.push_back(new Acceptor(io_, port, cb));
}

}