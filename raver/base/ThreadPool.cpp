#include "ThreadPool.h"
#include <cassert>
#include <iostream>
#include "Logger.h"

namespace raver {

ThreadPool::ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

ThreadPool::ThreadPool(size_t num_thread)
    : size_(num_thread), quit_(false), mtx_() {
  threads_.reserve(size_);
  for (size_t i = 0; i < size_; ++i) {
    threads_.emplace_back(&ThreadPool::worker, this);
  }
  LOG_TRACE << "ThreadPool ctor";
}

ThreadPool::~ThreadPool() {
  LOG_TRACE << "ThreadPool dtor";
  if (!quit_) {
    stop();
  }
}

void ThreadPool::stop() {
  {
    MutexGuard guard(mtx_);
    quit_ = true;
  }
  cv_.notify_all();
  for (auto& t : threads_) {
    assert(t.joinable());
    t.join();  // wait all threads.
  }
}

size_t ThreadPool::taskNum() const {
  MutexGuard guard(mtx_);
  return tasks_.size();
}

void ThreadPool::worker() {
  while (true) {
    TaskType task;
    {
      std::unique_lock<std::mutex> lock(mtx_);
      // if there are tasks in queue, pop one to current thread.
      cv_.wait(lock, [this]() { return quit_ || !tasks_.empty(); });
      if (quit_ && tasks_.empty()) {
        return;
      }

      task = std::move(tasks_.front());
      tasks_.pop();
    }
    LOG_TRACE << "task starts.";
    task();  // process task.
  }
}

}  // namespace raver
