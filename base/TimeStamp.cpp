#include "TimeStamp.h"

namespace raver {

typename TimeStamp::Ticks TimeStamp::getTicks()
{
    uint32_t hi;
    uint32_t lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return (uint64_t)hi << 32 | lo;
}

namespace {

static double ticks_per_second = 0;

void init()
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 500000000; // 500ms.

    TimeStamp::Ticks before = TimeStamp::getTicks();
    nanosleep(&ts, NULL);
    TimeStamp::Ticks after = TimeStamp::getTicks();
    ticks_per_second = (after - before) * 2;
}

}

pthread_once_t TimeStamp::once_control_ = PTHREAD_ONCE_INIT;

double TimeStamp::ticksPerSecond()
{
    pthread_once(&once_control_, &init);
    return ticks_per_second;
}

}
