#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"

namespace raver {

class Channel;

class Poller : noncopyable {
public:
    enum PollEvent {
        ERROR = 0x00000001,
        READ  = 0x00000002,
        WRITE = 0x00000003
    };

    Poller();

    ~Poller();

    void create();

    void setEvent(int fd, Channel* data);

    void getEvent(int i, int* events, Channel** data);

    int poll();

private:
    struct InternalPoller;
    InternalPoller* poller_;
};


}

#endif
