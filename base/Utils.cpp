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
        LOG_SYSERR << "socket";
    }
    return fd;
}

void setNonBlockAndCloseOnExec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags = flags | O_NONBLOCK;
    if ((::fcntl(sockfd, F_SETFL, flags)) == -1) {
        LOG_SYSERR << "fcntl error - set O_NONBLOCK";
    }

    flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= FD_CLOEXEC;
    if (::fcntl(sockfd, F_SETFL, flags) == -1) {
        LOG_SYSERR << "fcntl error - set FD_CLOEXEC";
    }
}

void setKeepAlive(int sockfd, bool on)
{
    int opt = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt,
            static_cast<socklen_t>(sizeof(opt))) < 0) {
        LOG_ERROR << "setsockopt error";
    }
}

void connect(int sockfd, const struct sockaddr* addr)
{
    if (::connect(sockfd, addr,
                    static_cast<socklen_t>(sizeof(struct sockaddr_in6))) == -1) {
        LOG_SYSFATAL << "connect" << strerror(errno);
    }
}

void bindOrDie(int sockfd, const struct sockaddr* addr)
{
    if (::bind(sockfd, addr,
                static_cast<socklen_t>(sizeof(struct sockaddr_in6))) < 0) {
        LOG_SYSFATAL << "bindOrDie";
    }
}

void listenOrDie(int sockfd)
{
    if (::listen(sockfd, SOMAXCONN) < 0) {
        LOG_SYSFATAL << "listenOrDie";
    }
}

int accept(int sockfd, struct sockaddr_in6* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    LOG_TRACE << "listen fd: " << sockfd;
    int connfd = ::accept(sockfd,
                static_cast<struct sockaddr*>(static_cast<void*>(addr)), &addrlen);
    //setNonBlockAndCloseOnExec(connfd);
    if (connfd < 0) {
        auto saved_errno = errno;
        LOG_TRACE << "accept";
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
                LOG_SYSERR << "unexpected error of accpet.";
                break;
            default:
                LOG_SYSERR << "unknown error of ::accept.";
                break;
        }
    }
    return connfd;
}

void close(int sockfd)
{
    if (::close(sockfd) == -1) {
        LOG_SYSERR << "close error";
    }
}

ssize_t read(int sockfd, void* buf, size_t count)
{
    ssize_t n;
    if ((n = ::read(sockfd, buf, count)) == -1) {
        LOG_SYSERR << "read error";
    }
    return n;
}

ssize_t write(int sockfd, const void* buf, size_t count)
{
    ssize_t n;
    if ((n = ::write(sockfd, buf, count)) == -1) {
        LOG_SYSERR << "write error";
    }
    return n;
}

int epoll_create(int size)
{
    int fd;
    if ((fd = ::epoll_create(size)) < 0) {
        LOG_SYSFATAL << "epoll_create error";
    }
    return fd;
}

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
{
    int nfds;
    if ((nfds = ::epoll_wait(epfd, events, maxevents, timeout)) < 0) {
        if (errno != EINTR) {
            LOG_SYSERR << "epoll_wait error";
        }
    }
    return nfds;
}

}

namespace utils {

std::string getFileExtension(const std::string& path)
{
    size_t pos;
    if ((pos = path.find_last_of('.')) != std::string::npos) {
        return std::string(path, pos);
    }
    return "";
}

void getContentType(const std::string& extension, std::string& content_type)
{
    content_type.clear();
    std::ifstream mimefile("mime.types");
    std::string line;
    LOG_INFO << "start";
    while (getline(mimefile, line)) {
        LOG_INFO << line;
        if (line[0] != '=') {
            std::stringstream line_stream(line);

            content_type.clear();
            line_stream >> content_type;
            LOG_INFO << "cont:" << content_type;
            std::string ext;
            while (line_stream >> ext) {
                if (ext == extension) {
                    return ;
                }
            }
        }
    }
    content_type = "text/plain";
}

}
