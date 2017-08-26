#include "../ThreadPool.h"
#include "../Logger.h"

using namespace raver;

std::mutex mtx;

// monitor all thread in threadpool for secs seconds.
void threadMonitor(const ThreadPool& pool, int secs)
{
    for (int i = 0; i < secs; ++i) {
        {
            MutexGuard guard(mtx);
            LOG_INFO << "size: " << pool.taskNum();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // wait 1 s.
    }
}

void task(int taskid)
{
    {
        MutexGuard guard(mtx);
        LOG_INFO << "task " << taskid << " processed.";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
    ThreadPool pool(10);
    pool.addTask(threadMonitor, std::ref(pool), 10);

    for (int i = 0; i < 1000; ++i) {
        pool.addTask(task, i);
    }
    //pool.stop();
}
