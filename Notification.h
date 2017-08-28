#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <mutex>
#include <condition_variable>

namespace raver {

class Notification {
public:
    Notification();

    void notify();

    void wait();

private:
    volatile bool notified_;
    std::mutex mtx_;
    std::condition_variable cv_;
};


}

#endif
