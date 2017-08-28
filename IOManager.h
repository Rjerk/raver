#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "ThreadPool.h"

namespace raver {

class Channel;
using Callback = std::function<void ()>;

class IOManager : noncopyable {
    friend class Channel;
public:
    explicit IOManager(int num_thread);

    ~IOManager();

    void poll();

    void stop();

    void addTask(TaskType task);

    Channel* newChannel(int listenfd, const Callback& readcb, const Callback& writecb);
    void removeChannel(Channel* ch);
private:
    ThreadPool* pool_;
};

}

#endif
