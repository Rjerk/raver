#include "ServiceManager.h"
#include "Acceptor.h"
#include "IOManager.h"
#include "../base/Logger.h"

#include <signal.h>

namespace raver {

namespace {

class IgnoreSigPipe {
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe init_obj;

}


ServiceManager::ServiceManager(int thread_num)
    : iomanager_(new IOManager{thread_num}), acceptor_{nullptr}
{
    LOG_TRACE << "ServiceManager ctor";
}


ServiceManager::~ServiceManager()
{
    LOG_TRACE << "ServiceManager dtor";
}

void ServiceManager::run()
{
    iomanager_->run();
}

void ServiceManager::registerAcceptor(int port, const AcceptorCallback& acceptor_cb)
{
    acceptor_.reset(new Acceptor(iomanager_.get(), port, acceptor_cb));
}
}
