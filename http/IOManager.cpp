#include "IOManager.h"
#include "Poller.h"
#include "Channel.h"
#include "../base/ThreadPool.h"
#include "../base/TimeStamp.h"
#include "../base/Logger.h"

#include <signal.h>

namespace raver {

namespace {

class IgnoreSigPipe {
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe initObj;

}

IOManager::IOManager(int num_thread)
    : pool_(new ThreadPool(num_thread)),
      poller_(new Poller()),
      channel_(),
      polling_(false),
      stopped_(false)
{
    LOG_TRACE << "IOManager ctor";
}

IOManager::~IOManager()
{
    LOG_TRACE << "IOManager dtor";
    stop();
}

void IOManager::addTask(const TaskType& task)
{
    pool_->addTask(task);
}

bool IOManager::stopped() const
{
    MutexGuard guard(mtx_stop_);
    return stopped_;
}

void IOManager::poll()
{
    {
        MutexGuard guard(mtx_stop_);
        polling_ = true;
    }

    while (!stopped()) {
        // got events.
        int nready = poller_->poll();

        int event_flags;
        for (int i = 0; i < nready; ++i) {
            Channel* ch = nullptr;
            poller_->getEvent(i, &event_flags, &ch);
            if (event_flags & (Poller::PollEvent::READ | Poller::PollEvent::ERROR)) {
                LOG_TRACE << "got readable event";
                ch->read();
            }
            if (event_flags & (Poller::PollEvent::WRITE | Poller::PollEvent::ERROR)) {
                LOG_TRACE << "got writable event";
                ch->write();
            }
        }
    }

    {
        MutexGuard guard(mtx_stop_);
        polling_ = false;
    }
    cv_polling_.notify_all();
}

void IOManager::stop()
{
    {
        MutexGuard guard(mtx_stop_);
        if (stopped()) {
            return ;
        }
        stopped_ = true;
    }

    // stop poller_;
    {
        std::unique_lock<std::mutex> lock(mtx_stop_);
        cv_polling_.wait(lock, [&](){ return polling_ == false; });
    }

    pool_->stop();
}

void IOManager::addTimer(double delay, const TaskType& task)
{
    TimeStamp::Ticks ts = TimeStamp::getTicks()
                     + delay * TimeStamp::ticksPerSecond();
    {
        MutexGuard guard(mtx_timer_queue_);
        timer_queue_.insert(std::make_pair(ts, task)); // process task at ts.
    }
}

Channel* IOManager::newChannel(int listenfd, const Callback& readcb, const Callback& writecb)
{
    Channel* new_ch = new Channel(this, listenfd, readcb, writecb);
    channel_.push_back(new_ch);
    poller_->setEvent(listenfd, new_ch);
    return new_ch;
}

void IOManager::removeChannel(Channel* ch)
{
    if (ch == nullptr) {
        return ;
    }
    {
        MutexGuard guard(mtx_channel_);
        channel_.remove(ch);
    }
}

}
