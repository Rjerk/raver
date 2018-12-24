#include "raver/base/Buffer.h"
#include <cassert>
#include <cstdio>
#include <unistd.h>

int main()
{
    using namespace raver;

    Buffer buf;

    assert(buf.size() == 8 + 1024);
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == 1024);

    buf.append("123", 3);
    assert(buf.readableBytes() == 3);
    assert(buf.writableBytes() == 1021);

    std::string a = "1234567890";
    buf.append(a);
    assert(buf.readableBytes() == 13);
    assert(buf.writableBytes() == 1011);

    int saveno;

    //char message[1024] = {0};
    char message[10240] = {0};
    int pipefd[2];
    ::pipe(pipefd);

    LOG_INFO << "pipe";

    int pid;
    if ((pid = fork()) == 0) {
        ::close(pipefd[0]);
        auto sz = ::write(pipefd[1], message, sizeof(message));
        LOG_INFO << "child process " << "write " << sz << " bytes.";
        exit(0);
    } else {
        ::close(pipefd[1]);
        LOG_INFO << "parent process " << "read " << buf.readFd(pipefd[0], &saveno) << " bytes.";
    }

}
