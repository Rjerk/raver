#include "IOManager.h"

#include "ThreadPool.h"

namespace raver {

IOManager::IOManager(int num_thread)
    : pool_(new ThreadPool(num_thread))
{
}

IOManager::~IOManager()
{
}

void IOManager::addTask(TaskType task)
{
    pool_->addTask(task);
}

void IOManager::stop()
{
    pool_->stop();
}



}
