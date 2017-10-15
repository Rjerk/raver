#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "../base/noncopyable.h"

#include <functional>
#include <memory>
#include <vector>

namespace raver {

class Channel;
class ThreadPool;
class EPoller;
class ServiceManager;

class IOManager {
public:
    using ReadCallback = std::function<void ()>;
    using WriteCallback = std::function<void ()>;
    using TaskType = std::function<void ()>;

    IOManager(int thread_num, ServiceManager* service);

    ~IOManager();

    void run();

    void removeChannel(Channel* channel);

    void updateChannel(Channel* channel);

    void addTask(const TaskType& task);

private:
    ServiceManager* service_manager_; // not own it.
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<EPoller> epoller_;

    bool event_handling_;
    bool quit_;

    using ChannelList = std::vector<Channel*>;
    ChannelList active_channels_;
    Channel* current_active_channel_;

};


}


#endif
