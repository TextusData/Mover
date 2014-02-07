/* Condition.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 * A wrapper around the pthreads condition class.
 */

#ifndef TEXTUS_BASE_LOCKS_CONDITION_H_
#define TEXTUS_BASE_LOCKS_CONDITION_H_

#include <pthread.h>
#include <string.h> // for memset.
#include <errno.h>

#include "textus/base/locks/Mutex.h"
#include "textus/base/Time.h"
#include "textus/base/Compiler.h"




namespace textus { namespace base { 

extern bool threadWaiting(textus::base::locks::Mutex *mutex);

namespace locks  {

using namespace textus::base;

class Condition /* This is a lightweight class, so no base.  We build base on top of this. */
{
private:
  pthread_cond_t cond;

public:
  Condition() {
    memset(&cond, 0, sizeof(cond));
    pthread_cond_init(&cond, NULL);
  }

  ~Condition()
  {
    // wake up anyone that might be listening.
    pthread_cond_broadcast(&cond);
    pthread_cond_destroy(&cond);
  }

  void signal()
  {
    mb();
    pthread_cond_broadcast(&cond);
  }

  void signalOne()
  {
    mb();
    pthread_cond_signal(&cond);
  }

  // Mutex must be locked and unlocked by the caller.
  void wait(Mutex &mutex)
  {
    if (!threadWaiting(&mutex)) {
      int ret = pthread_cond_wait(&cond, &mutex.mutex);
      assert (ret == 0);
    }
    mb();
  }

  void waitTimeOut(Mutex &mutex, const Time &timeout)
  {
    struct timespec ts;
    int ret=0;
    timeout.localts(&ts);
    if (!threadWaiting(&mutex)) {
      ret = pthread_cond_timedwait(&cond, &mutex.mutex, &ts);
    }
    mb();
    if (ret != 0 && ret != ETIMEDOUT) {
      LOG(WARNING) << "pthread_cond_timedwait returned: " << ret <<", errno = " << errno << "\n";
    }
  }
};
}}} // namespace

#endif //TEXTUS_BASE_LOCKS_CONDITION_H_
