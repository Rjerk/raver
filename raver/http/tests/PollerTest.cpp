#include <raver/http/EPoller.h>
#include <raver/http/Channel.h>
#include <raver/base/Logger.h>
#include <raver/base/Utils.h>
#include <raver/http/IOManager.h>
#include <cstring>

int main()
{
    using namespace raver;

    int port = 8888;
    int listenfd = utils::socket(AF_INET, SOCK_STREAM, 0);

    LOG_INFO << "listenfd: " << listenfd;

    struct sockaddr_in servaddr;
    ::bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    int ret = ::setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));
    if (ret < 0 && opt) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
    ::setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (const void*)&opt, sizeof(opt));

    utils::bindOrDie(listenfd, (struct sockaddr*) &servaddr);
    utils::listenOrDie(listenfd);

    struct sockaddr_in clntaddr;
    socklen_t len = sizeof(clntaddr);
    int conn = ::accept(listenfd, (struct sockaddr*) &clntaddr, &len);
    if (conn > 0) {
        LOG_INFO << "accept";
    } else {
        LOG_SYSFATAL << "error" << ::strerror(errno);
    }

    LOG_INFO << "connfd: " << conn;

    Poller poller;

    poller.setEvent(conn, nullptr);

    int event_num = poller.poll();

    LOG_INFO << "events: " << event_num;
}
