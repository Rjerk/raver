#ifndef RW_MUTEX_H
#define RW_MUTEX_H

#include <pthread.h>
#include "noncopyable.h"

namespace raver {

class RWMutex : noncopyable {
 public:
  RWMutex() { pthread_rwlock_init(&rw_lock_, nullptr); }
  ~RWMutex() { pthread_rwlock_destroy(&rw_lock_); }
  void rdlock() { pthread_rwlock_rdlock(&rw_lock_); }
  void wrlock() { pthread_rwlock_wrlock(&rw_lock_); }
  void unlock() { pthread_rwlock_unlock(&rw_lock_); }

 private:
  pthread_rwlock_t rw_lock_;
};

}  // namespace raver

#endif
