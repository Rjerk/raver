#ifndef POLLER_H
#define POLLER_H

#include "../base/noncopyable.h"
#include <memory>

namespace raver {

class Channel;

class Poller : noncopyable {
public:
    enum PollEvent {
        ERROR = 0x00000001,
        READ  = 0x00000002,
        WRITE = 0x00000004
    };

    Poller();

    ~Poller();

    void setEvent(int fd, Channel* data);

    void getEvent(int i, int* events, Channel** data);

    int poll();

private:
    struct InternalPoller;
    std::unique_ptr<InternalPoller> poller_;
};


}

#endif
