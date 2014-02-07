/* LeakyBucket.h -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 *
 * Template for a leaky bucket to time things.
 * properly.
 */

#ifndef TEXTUS_BASE_LEAKY_BUCKET_H_
#define TEXTUS_BASE_LEAKY_BUCKET_H_

#include "textus/base/Base.h"


namespace textus { namespace base {

using namespace textus::base;

extern  uint32_t leakyTimeFunc(void);
typedef uint32_t (*leakyTimeFuncType)();

template <class T, leakyTimeFuncType timefunc=leakyTimeFunc, T initial_value = 0 > class LeakyBucket: virtual public Base {
private:
  T value;
  time_t last_check;
  bool overflow_;
  T limit;
  T rate_n;
  T rate_d;

protected:
  void leak() {
    time_t t = timefunc();
    Synchronized(this);
    if (t - last_check > 0) {
      value -= (t - last_check) * rate_n / rate_d;
      if (value < initial_value) {
	value = initial_value;
      }
      last_check = t;
    }
  }

public:
  explicit LeakyBucket(): value(initial_value), last_check(timefunc()), overflow_(false),limit(0) {}
  explicit LeakyBucket(T l): value(initial_value), last_check(timefunc()), overflow_(false), limit(l) {}

  void setOverflow(bool b) {
    overflow_ = b;
  }

  LeakyBucket &operator ++() {
    Synchronized(this);
    leak();
    value++;
    if (value > limit) {
      value = limit;
      setOverflow(true);
    }
    return *this;
  }

  LeakyBucket &operator +(const T &a) {
    Synchronized(this);
    leak();
    value += a;
    if (value > limit) {
      value = limit;
      setOverflow(true);
    }
    return *this;
  }

  void setRate(T n, T d) {
    rate_n = n;
    rate_d = d;
  }

  bool overflow() {
    Synchronized(this);
    bool o = overflow_;
    setOverflow(false);
    return o;
  }

  void reset() {
    Syncrhonized(this);
    overflow = false;
    value = initial_value;
  }

};

}} //namespace

#endif
