#include "Channel.h"
#include "IOManager.h"

#include <unistd.h>
#include <fcntl.h>

namespace raver {

void Channel::readWhenReady()
{

}

void Channel::writeWhenReady()
{
}


Channel::Channel(IOManager* io, int fd,
                 const Callback& readcb,
                 const Callback& writecb)
    : fd_(fd), io_(io), closed_fd_(false),
      readcb_(readcb), writecb_(writecb), mtx_(), next(nullptr)
{
    int flags = ::fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

Channel::~Channel()
{
}

ThreadPool* Channel::getPool()
{
    return io_->pool_;
}

}
