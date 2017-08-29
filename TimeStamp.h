#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include "noncopyable.h"

#include <pthread.h>
#include <inttypes.h>

namespace raver {

class TimeStamp : noncopyable {
public:
    using Ticks = uint64_t;

    TimeStamp() {}
    ~TimeStamp() {}

    static Ticks getTicks();

    static double ticksPerSecond();

private:
    static pthread_once_t once_control_;
};

}

#endif
