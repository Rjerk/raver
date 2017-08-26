#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>

namespace wrapper {

int socket(int domain, int type, int protocol);

void connect(int sockfd, const struct sockaddr* addr);

void bindOrDie(int sockfd, const struct sockaddr* addr);

void listenOrDie(int sockfd);

int accept(int sockfd, struct sockaddr_in6* addr);

ssize_t read(int sockfd, void* buf, size_t count);

ssize_t write(int sockfd, const void* buf, size_t count);

void close(int sockfd);

void setNonBlockAndCloseOnExec(int sockfd);

int epoll_create(int size);

int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);

}


#endif
