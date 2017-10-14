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

class IOManager {
public:
    using ReadCallback = std::function<void ()>;
    using WriteCallback = std::function<void ()>;
    using TaskType = std::function<void ()>;

    explicit IOManager(int thread_num);

    ~IOManager();

    void run();

    void removeChannel(Channel* channel);

    void updateChannel(Channel* channel);

    void addTask(const TaskType& task);

private:
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
