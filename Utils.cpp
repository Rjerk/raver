#include "Utils.h"
#include "Logger.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>

namespace wrapper {

int socket(int domain, int type, int protocol)
{
    int fd;
    if ((fd = ::socket(domain, type, protocol)) < 0) {
        LOG_ERROR << "socket";
    }
    return fd;
}

void setNonBlockAndCloseOnExec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags = flags | O_NONBLOCK;
    if ((::fcntl(sockfd, F_SETFL, flags)) == -1) {
        LOG_ERROR << "setNonBlockAndCloseOnExec: fcntl error - set O_NONBLOCK";
    }

    flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= FD_CLOEXEC;
    if (::fcntl(sockfd, F_SETFL, flags) == -1) {
        LOG_ERROR << "setNonBlockAndCloseOnExec: fcntl error - set FD_CLOEXEC";
    }
}

void connect(int sockfd, const struct sockaddr* addr)
{
    if (::connect(sockfd, addr,
                    static_cast<socklen_t>(sizeof(struct sockaddr_in6))) == -1) {
        LOG_FATAL << "connect" << strerror(errno);
    }
}

void bindOrDie(int sockfd, const struct sockaddr* addr)
{
    if (::bind(sockfd, addr,
                static_cast<socklen_t>(sizeof(struct sockaddr_in6))) < 0) {
        LOG_FATAL << "bindOrDie";
    }
}

void listenOrDie(int sockfd)
{
    if (::listen(sockfd, SOMAXCONN) < 0) {
        LOG_FATAL << "listenOrDie";
    }
}

int accept(int sockfd, struct sockaddr_in6* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = ::accept(sockfd,
                static_cast<struct sockaddr*>((void*)addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
    if (connfd < 0) {
        auto saved_errno = errno;
        LOG_ERROR << "accept";
        switch (saved_errno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_FATAL << "unexpected error of ::accpet: "
                          << saved_errno << ::strerror(saved_errno);
                break;
            default:
                LOG_FATAL << "unknown error of ::accept: " << saved_errno;
                break;
        }
    }
    return connfd;
}

void close(int sockfd)
{
    if (::close(sockfd) == -1) {
        LOG_ERROR << "close";
    }
}

ssize_t read(int sockfd, void* buf, size_t count)
{
    ssize_t n;
    if ((n = ::read(sockfd, buf, count)) == -1) {
        LOG_ERROR << "read";
    }
    return n;
}

ssize_t write(int sockfd, const void* buf, size_t count)
{
    ssize_t n;
    if ((n = ::write(sockfd, buf, count)) == -1) {
        LOG_ERROR << "write";
    }
    return n;
}

int epoll_create(int size)
{
    int fd;
    if ((fd = ::epoll_create(size)) < 0) {
        LOG_ERROR << "epoll_create";
    }
    return fd;
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)
{

}

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
{
    int nfds;
    if ((nfds = ::epoll_wait(epfd, events, maxevents, timeout) < 0) {
        if (errno != EINTR) {
            LOG_ERROR << "epoll_wait";
        }
    }
    return nfds;
}

}
