#include "Channel.h"
#include "IOManager.h"
#include "Utils.h"

namespace raver {

// if socket can be read.
void Channel::readWhenReady()
{
    bool ready = false;
    {
        MutexGuard guard(mtx_);
        if (can_read_) {
            ready = true;
            can_read_ = false;
        } else {
            waiting_read_ = true;
        }
    }
    if (ready) {
        getPool()->addTask(readcb_);
    }
}


// if socket wants to be read.
void Channel::readIfWaiting()
{
    bool ready = false;
    {
        MutexGuard guard(mtx_);
        if (waiting_read_) {
            ready = true;
            waiting_read_ = false;
        } else {
            can_read_ = true;
        }
    }
    if (ready) {
        getPool()->addTask(readcb_);
    }
}

// if socket can be writed.
void Channel::writeWhenReady()
{
    getPool()->addTask(writecb_);
}

void Channel::writeIfWaiting()
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
}

Channel::~Channel()
{
}

ThreadPool* Channel::getPool()
{
    return io_->pool_;
}

}
