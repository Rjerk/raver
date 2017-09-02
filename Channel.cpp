#include "Channel.h"
#include "IOManager.h"
#include "Utils.h"

namespace raver {

void Channel::readWhenReady()
{
    LOG_INFO << "readWhenReady";
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

void Channel::writeWhenReady()
{
    LOG_INFO << "writeWhenReady";
    bool ready = false;
    {
        MutexGuard guard(mtx_);
        if (can_write_) {
            ready = true;
            can_write_ = false;
        } else {
            waiting_write_ = true;
        }
    }
    if (ready) {
        getPool()->addTask(writecb_);
    }
}

Channel::Channel(IOManager* io, int fd,
                 const Callback& readcb,
                 const Callback& writecb)
    : fd_(fd), io_(io), closed_fd_(false),
      readcb_(readcb), writecb_(writecb), mtx_(), next_(nullptr),
      can_read_(false), can_write_(false), waiting_read_(false), waiting_write_(false)
{
    wrapper::setNonBlockAndCloseOnExec(fd_);
}

Channel::~Channel()
{
}

void Channel::readIfWaiting()
{
    LOG_INFO << "readIfWaiting";
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

void Channel::writeIfWaiting()
{
    LOG_INFO << "writeIfWaiting";
    bool ready = false;
    {
        MutexGuard guard(mtx_);
        if (waiting_write_) {
            ready = true;
            waiting_write_ = false;
        } else {
            can_write_ = true;
        }
    }
    if (ready) {
        getPool()->addTask(writecb_);
    }
}

ThreadPool* Channel::getPool()
{
    return io_->pool_;
}

}
