#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "noncopyable.h"

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>
#include <memory>
#include <queue>

namespace raver {

using TaskType = std::function<void ()>;
using MutexGuard = std::lock_guard<std::mutex>;

class ThreadPool : noncopyable {
public:
    ThreadPool();

    explicit ThreadPool(size_t num_thread);

    ~ThreadPool();

    template <typename Func, typename... Args>
    void addTask(Func&& func, Args&&... args)
	{
		auto execute = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
		using ReturnType = typename std::result_of<Func(Args...)>::type;
	    using PackagedTask = std::packaged_task<ReturnType ()>;
	    auto task = std::make_shared<PackagedTask>(std::move(execute));
	    {
	        MutexGuard guard(mtx_);
	        tasks_.emplace([task]() { (*task)(); });
	    }
	    cv_.notify_one(); // notify a thread to process the task.
	}

    void stop();

    size_t taskNum() const;

private:
    void worker();

private:
    size_t size_;
    bool quit_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::vector<std::thread> threads_;
    std::queue<TaskType> tasks_;
};

}

#endif
