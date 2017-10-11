#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "../base/ThreadPool.h"
#include "../base/TimeStamp.h"
#include "Poller.h"
#include <map>
#include <list>

namespace raver {

class Channel;
class Poller;

using Callback = std::function<void ()>;

class IOManager : noncopyable {
    friend class Channel;
public:
    explicit IOManager(int num_thread);

    ~IOManager();

    void poll();

    void stop();

    void addTask(const TaskType& task);

    void addTimer(double delay, const TaskType& task);

    Channel* newChannel(int listenfd, const Callback& readcb, const Callback& writecb);

    void removeChannel(Channel* ch);

    bool stopped() const;

    Poller* poller() const { return poller_.get(); }
private:
    std::unique_ptr<ThreadPool> pool_; // own it.
    std::unique_ptr<Poller> poller_; // own it.
    std::list<Channel*> channel_; //own it.
    bool polling_;
    bool stopped_;

    using TimerQueue = std::multimap<TimeStamp::Ticks, TaskType>;
    TimerQueue timer_queue_;

    std::mutex mtx_timer_queue_;
    mutable std::mutex mtx_stop_;
    std::mutex mtx_channel_;
    std::condition_variable cv_polling_;
};

}

#endif
