#include "IOManager.h"
#include "../base/Logger.h"
#include "../base/ThreadPool.h"
#include "Channel.h"
#include "EPoller.h"
#include "ServiceManager.h"

#include <algorithm>
#include <cassert>

namespace raver {

IOManager::IOManager(int thread_num, ServiceManager* service)
    : service_manager_(service),
      threadpool_(new ThreadPool(thread_num)),
      epoller_(new EPoller(this)),
      event_handling_(false),
      quit_(false),
      active_channels_(),
      current_active_channel_(nullptr) {
  LOG_TRACE << "IOManager ctor.";
}

IOManager::~IOManager() { LOG_TRACE << "IOManager dtor."; }

void IOManager::run() {
  LOG_TRACE << "event handing begin.";
  while (!quit_) {
    active_channels_.clear();
    epoller_->poll(&active_channels_);

    event_handling_ = true;
    for (auto& active_channel : active_channels_) {
      LOG_TRACE << "handling active channel.";
      current_active_channel_ = active_channel;
      current_active_channel_->handleEvent();
    }
    current_active_channel_ = nullptr;
    event_handling_ = false;
  }

  LOG_TRACE << "IOManager stopped.";
}

void IOManager::updateChannel(Channel* channel) {
  epoller_->updateChannel(channel);
}

void IOManager::removeChannel(Channel* channel) {
  if (event_handling_) {
    assert(current_active_channel_ == channel ||
           std::find(active_channels_.begin(), active_channels_.end(),
                     channel) == active_channels_.end());
  }
  epoller_->removeChannel(channel);
}

void IOManager::addTask(const TaskType& task) { threadpool_->addTask(task); }

}  // namespace raver
