#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string>
#include <sstream>
#include <fstream>
#include <set>

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

void setKeepAlive(int sockfd, bool on);

int epoll_create(int size);

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);

}

namespace utils {

std::string getFileExtension(const std::string& path);

void getContentType(const std::string& extension, std::string& content_type);

}


#endif
