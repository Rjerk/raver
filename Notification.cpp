#include "Notification.h"
//#include <iostream>
//#include <thread>
//#include <chrono>

namespace raver {

Notification::Notification()
    : notified_{false}
{
}

void Notification::wait()
{
    std::unique_lock<std::mutex> lock(mtx_);
    //std::cout << "wait...";
    cv_.wait(lock, [&]{ return notified_ == true; });
    //std::cout << "got it...\n";
}

void Notification::notify()
{
    {
        std::lock_guard<std::mutex> guard(mtx_);
        notified_ = true;
        //std::this_thread::sleep_for(std::chrono::seconds(3));
        //std::cout << "notifing...\n";
    }
    cv_.notify_all();
}

}
