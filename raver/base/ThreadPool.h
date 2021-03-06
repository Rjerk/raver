#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "Logger.h"
#include "noncopyable.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace raver {

using TaskType = std::function<void()>;
using MutexGuard = std::lock_guard<std::mutex>;

class ThreadPool : noncopyable {
 public:
  ThreadPool():
    ThreadPool(std::thread::hardware_concurrency() ?
                std::thread::hardware_concurrency() : 1) {}

  explicit ThreadPool(size_t num_thread);

  ~ThreadPool();

  template <typename Func, typename... Args>
  void addTask(Func&& func, Args&&... args) {
    LOG_TRACE << "add_task begin";

    auto execute = std::bind(std::forward<Func>(func),
                        std::forward<Args>(args)...);

    using ReturnType = typename std::result_of<Func(Args...)>::type;
    using PackagedTask = std::packaged_task<ReturnType()>;

    auto task = std::make_shared<PackagedTask>(std::move(execute));

    {
      MutexGuard guard(mtx_);
      tasks_.emplace([task]() { (*task)(); });
    }

    LOG_TRACE << "before notify";
    cv_.notify_one();  // notify a thread to process the task.
    LOG_TRACE << "add_task end";
  }

  void stop();

  size_t TaskNum() const;

 private:
  void Worker();

 private:
  bool quit_{false};
  mutable std::mutex mtx_;
  std::condition_variable cv_;

  std::queue<TaskType> tasks_;
  size_t size_;
  std::vector<std::thread> threads_;
};

}  // namespace raver

#endif
