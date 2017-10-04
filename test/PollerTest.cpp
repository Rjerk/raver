#include "../http/Poller.h"
#include "../http/Channel.h"
#include "../base/Logger.h"
#include "../base/Utils.h"
#include <cstring>

int main()
{
    using namespace raver;

    int port = 8888;
    int listenfd = wrapper::socket(AF_INET, SOCK_STREAM, 0);

    wrapper::setNonBlockAndCloseOnExec(listenfd);

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

    wrapper::bindOrDie(listenfd, (struct sockaddr*) &servaddr);
    wrapper::listenOrDie(listenfd);


    Poller poller;

    poller.create();

    for (; ; ) {

        int nready = poller.poll();

        struct sockaddr_in clntaddr;
        socklen_t len = sizeof(clntaddr);
        int conn = ::accept(listenfd, (struct sockaddr*) &clntaddr, &len);
        if (conn > 0) {
            LOG_INFO << "accept";
        } else {
            LOG_INFO << "error";
            continue;
        }


        char buf[1024];
        int n = wrapper::read(conn, buf, sizeof(buf));
        LOG_INFO << n << "\n" << buf;

        int event_flags = 0;
        for (int i = 0; i < nready; ++i) {

            LOG_INFO << "events num: " << nready;

            Channel* ch = nullptr;
            poller.getEvent(i, &event_flags, &ch);

            if (event_flags & (Poller::PollEvent::READ | Poller::PollEvent::ERROR)) {
                LOG_INFO << "readable event.";
                ch->read();
            }

            if (event_flags & (Poller::PollEvent::WRITE | Poller::PollEvent::ERROR)) {
                LOG_INFO << "writable event.";
                ch->write();
            }
        }
    }
}
