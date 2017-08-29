#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

namespace raver {

class Timer {
public:
    Timer();

    ~Timer();

    void start();

    void end();

    void reset();

    double elapsed() const;

private:
    struct timeval start_;
    struct timeval end_;
};


}


#endif
