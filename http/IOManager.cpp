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
      channel_(nullptr),
      polling_(false),
      stopped_(false)
{
    LOG_DEBUG << "IOManager ctor";
    poller_->create();
}

IOManager::~IOManager()
{
    LOG_DEBUG << "IOManager dtor";
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

        {
            MutexGuard guard(mtx_timer_queue_);
            TimeStamp::Ticks now = TimeStamp::getTicks();

            auto to_execute = timer_queue_.cbegin();
            while (to_execute != timer_queue_.cend()) {
                if (to_execute->first > now) {
                    break;
                } else {
                    // handle tasks in timer queue.
                    pool_->addTask(to_execute->second);
                    timer_queue_.erase(to_execute++);
                }
            }
        }

        int event_flags;
        for (int i = 0; i < nready; ++i) {
            LOG_DEBUG << "events num: " << nready;
            Channel* ch = nullptr;
            poller_->getEvent(i, &event_flags, &ch);
            if (event_flags & (Poller::PollEvent::READ | Poller::PollEvent::ERROR)) {
                LOG_DEBUG << "got readable event";
                ch->readIfWaiting();
            }
            if (event_flags & (Poller::PollEvent::WRITE | Poller::PollEvent::ERROR)) {
                LOG_DEBUG << "got writable event";
                ch->writeIfWaiting();
            }
        }

        Channel* to_delete;
        {
            MutexGuard guard(mtx_channel_);
            to_delete = channel_;
            channel_ = nullptr;
        }
        while (to_delete) {
            auto prev = to_delete;
            to_delete = to_delete->next_;
            delete prev;
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

    while (channel_) {
        auto prev = channel_;
        channel_ = channel_->next_;
        delete prev;
    }
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
        ch->next_ = channel_;
        channel_ = ch;
        ch = nullptr;
    }
}

}
