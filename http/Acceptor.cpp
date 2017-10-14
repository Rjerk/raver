#include "../base/Utils.h"
#include "../base/Logger.h"
#include "Acceptor.h"
#include "IOManager.h"
#include "Channel.h"

#include <cstring>

namespace raver {

Acceptor::Acceptor(IOManager* iomanager, int port, const AcceptorCallback& acceptor_cb)
    : iomanager_(iomanager), listenfd_(-1), channel_(), acceptor_cb_(acceptor_cb)
{
    listenfd_ = wrapper::socket(AF_INET, SOCK_STREAM, 0);

    wrapper::setNonBlockAndCloseOnExec(listenfd_);

    struct sockaddr_in serv_addr;
    ::bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    ::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));
    ::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEPORT, (const void*)&opt, sizeof(opt));

    wrapper::bindOrDie(listenfd_, (struct sockaddr*) &serv_addr);
    wrapper::listenOrDie(listenfd_);

    channel_.reset(new Channel(iomanager_, listenfd_));
    channel_->setReadCallback(std::bind(&Acceptor::doAccept, this));
}

Acceptor::~Acceptor()
{
    channel_->disableAll();
    channel_->remove();
    wrapper::close(listenfd_);
}

void Acceptor::doAccept()
{
    for ( ; ; ) {
        struct sockaddr_in clnt_addr;
        socklen_t len = sizeof(clnt_addr);
        int connfd = ::accept(listenfd_, (struct sockaddr *) &clnt_addr, &len);
        if (connfd == -1) {
            auto saved_errno = errno;
            switch (saved_errno) {
                case EINTR: // interrupted by a signal that was caught before a valid connection arrived.
                case ECONNABORTED: // a connection has been aborted.
                    continue;

                case EAGAIN: // same as EWOULDBLOCK, if no pending connections are present on the incomplete connection queue.
                    //channel_->readWhenReady();
                    break;

                case EBADF:
                case EFAULT:
                case EINVAL:
                case EMFILE:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                case EPROTO:
                case EPERM:
                    LOG_SYSERR << "unexpected error of accept.";
                    break;

                default: // various Linux kernels can return other errors.
                    LOG_SYSERR << "unknown error of accept.";
                    break;
            }
        }

        if (connfd >= 0) {
            if (acceptor_cb_) {
                acceptor_cb_(connfd); // HTTPService::newConnection(int connfd)
            } else {
                wrapper::close(connfd);
            }
        }
    }
}

}
