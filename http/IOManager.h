#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "ThreadPool.h"
#include "TimeStamp.h"
#include <map>

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
private:
    ThreadPool* pool_;
    Poller* poller_;
    Channel* channel_;
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
