#include "Channel.h"
#include "IOManager.h"
#include "../base/Utils.h"

namespace raver {

// if socket wants to be read.
void Channel::read()
{
    getPool()->addTask(readcb_);
}

// if socket can be writed.
void Channel::write()
{
    getPool()->addTask(writecb_);
}

Channel::Channel(IOManager* io, int fd,
                 const Callback& readcb,
                 const Callback& writecb)
    : fd_(fd), io_(io), closed_fd_(false),
      readcb_(readcb), writecb_(writecb), mtx_(), next_(nullptr),
      can_read_(false), can_write_(false), waiting_read_(false), waiting_write_(false)
{
    LOG_INFO << "Channel ctor";
    wrapper::setNonBlockAndCloseOnExec(fd_);
    wrapper::setKeepAlive(fd_, true);
}

Channel::~Channel()
{
}

ThreadPool* Channel::getPool()
{
    return io_->pool_.get();
}

}
