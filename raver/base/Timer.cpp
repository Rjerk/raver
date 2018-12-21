#include "Timer.h"

namespace raver {

Timer::Timer() { reset(); }

Timer::~Timer() = default;

void Timer::start() { ::gettimeofday(&start_, nullptr); }

void Timer::end() { ::gettimeofday(&end_, nullptr); }

void Timer::reset() {
  start_.tv_sec = 0;
  start_.tv_usec = 0;
  end_.tv_sec = 0;
  end_.tv_usec = 0;
}

double Timer::elapsed() const {
  double sec = 0;
  if (end_.tv_usec < start_.tv_usec) {
    sec = (static_cast<double>(end_.tv_sec - 1 - start_.tv_sec) +
           static_cast<double>(end_.tv_usec + 1000000 - start_.tv_usec)) /
          1000000.0;
  } else {
    sec = (static_cast<double>(end_.tv_sec - start_.tv_sec) +
           static_cast<double>(end_.tv_usec - start_.tv_usec)) /
          1000000.0;
  }
  return sec;
}

}  // namespace raver
