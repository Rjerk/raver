#include "../Notification.h"
#include <iostream>
#include <thread>
#include <unistd.h>

int main()
{
    raver::Notification n;

    std::thread t1{&raver::Notification::wait, &n};
    std::thread t2{&raver::Notification::wait, &n};
    std::thread t3{&raver::Notification::wait, &n};
    std::thread t4{&raver::Notification::wait, &n};

    std::thread s{&raver::Notification::notify, &n};

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    s.join();
}
