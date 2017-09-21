#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "noncopyable.h"
#include <functional>

namespace raver {

class IOManager;
class Channel;
using AcceptorCallback = std::function<void (int)>;

class Acceptor : noncopyable {
public:
    Acceptor(IOManager* io, int port, const AcceptorCallback& cb);

    ~Acceptor();

    void startAccept();

    void close();

private:
    void doAccept();
    void doNothing() {}

private:
    int listenfd_;
    IOManager* manager_;
    Channel* channel_;
    AcceptorCallback accept_cb_;
};

}

#endif
