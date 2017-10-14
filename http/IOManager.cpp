#include "../base/Logger.h"
#include "../base/ThreadPool.h"

#include "IOManager.h"
#include "Channel.h"
#include "EPoller.h"

#include <algorithm>
#include <cassert>

namespace raver {

IOManager::IOManager(int thread_num)
    : threadpool_(new ThreadPool(thread_num)),
      epoller_(new EPoller(this)),
      event_handling_(false),
      quit_(false),
      active_channels_(),
      current_active_channel_(nullptr)
{
    LOG_TRACE << "IOManager ctor.";
}

IOManager::~IOManager()
{
    LOG_TRACE << "IOManager dtor.";
}

void IOManager::run()
{
    while (!quit_) {
        active_channels_.clear();
        epoller_->poll(&active_channels_);

        event_handling_ = true;
        LOG_TRACE << "event handing begin.";
        for (auto it = active_channels_.begin(); it != active_channels_.end(); ++it) {
            current_active_channel_ = *it;
            current_active_channel_->handleEvent();
        }
        LOG_TRACE << "event handing end.";
        current_active_channel_ = nullptr;
        event_handling_ = false;
    }

    LOG_TRACE << "IOManager stopped.";
}

void IOManager::updateChannel(Channel* channel)
{
    epoller_->updateChannel(channel);
}

void IOManager::removeChannel(Channel* channel)
{
    if (event_handling_) {
        assert(current_active_channel_ == channel
            || std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
    }
    epoller_->removeChannel(channel);
}

void IOManager::addTask(const TaskType& task)
{
    threadpool_->addTask(task);
}


}
