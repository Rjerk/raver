#include "Acceptor.h"
#include "IOManager.h"
#include "Utils.h"
#include "Channel.h"
#include <strings.h>

namespace raver {

Acceptor::Acceptor(IOManager* io, int port, const AcceptorCallback& cb)
    : listenfd_(-1), io_(io), channel_(nullptr), accept_cb_(cb)
{
    // get and set listenfd.
    listenfd_ = wrapper::socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    ::bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int opt = 1;
    ::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));
    wrapper::bindOrDie(listenfd_, (struct sockaddr*) &servaddr);
    wrapper::listenOrDie(listenfd_);

    channel_ = io_->newChannel(listenfd_, std::bind(&Acceptor::doAccept, this),
                                          std::bind(&Acceptor::doNothing, this));
}

Acceptor::~Acceptor()
{
    if (io_) {
        close();
    }
}

void Acceptor::accept()
{
    channel_->readWhenReady();
}

void Acceptor::close()
{
    wrapper::close(listenfd_);
    if (channel_) {
        io_->removeChannel(channel_);
    }
    channel_ = nullptr;
}

void Acceptor::doAccept()
{
    for ( ; ; ) {
        struct sockaddr_in clntaddr;
        int fd = wrapper::accept(listenfd_, (struct sockaddr_in6 *) &clntaddr);
        accept_cb_(fd);
    }
}

}
