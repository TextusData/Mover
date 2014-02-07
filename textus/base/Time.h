/* Time.h -*- c++ -*-
 * Copyright (c) 2009, 2013 Ross Biro
 *
 * A class that represents the time.
 * WARNING: Assumes GMT for conversions and compatibility with 
 * database times.
 */

#ifndef TEXTUS_BASE_TIME_H_
#define TEXTUS_BASE_TIME_H_

#include "textus/base/Utility.h"

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <ostream>

namespace textus { namespace base {
class Duration;

class Time {
  friend class Duration;

private:
  long long t; //assumed to be in gmt.

public:
  static const int TicksPerSecond = 1000000;
  static Time now();
  explicit Time(time_t n): t(n * TicksPerSecond) {} // assumes n is in GMT.  Use below to convert.
  explicit Time(time_t n, const struct tm &tm): t((n - tm.tm_gmtoff + 3600*(tm.tm_isdst > 0?1:0)) * TicksPerSecond) {}
  explicit Time(struct tm *tm): t((mktime(tm) - tm->tm_gmtoff + 3600*(tm->tm_isdst > 0?1:0)) * TicksPerSecond) {}

  Time(): t(0) {}
  ~Time() {}

  static Time maxTime() {
    Time t;
    t.t = static_cast<long long>(INT_MAX)*TicksPerSecond;
    return t;
  }

  long long seconds() const {
    return t/Time::TicksPerSecond;
  }

  long long remainder_nanoseconds() const {
    return (1000000000/Time::TicksPerSecond)*(t % Time::TicksPerSecond);
  }

  Time operator + (const Duration &d) const;
  Time operator - (const Duration &d) const;
  
  bool operator <(const Time &t2) const
  {
    return ((t2.t - t) > 0);
  }

  bool before (const Time &t2) const
  {
    return *this < t2;
  }

  bool after (const Time &t2) const
  {
    return *this > t2;
  }

  bool operator ==(const Time &t2) const
  {
    return t == t2.t;
  }

  COMPARISON_OPERATORS(Time);

  Duration elapsedTime() const;

  std::string toString() const {
    char buff[30];
    snprintf(buff, sizeof(buff), "%lld.%09lld", seconds(), remainder_nanoseconds());
    return std::string(buff);
  }

  struct timespec *localts(struct timespec *ts) const {
    time_t tt = t/TicksPerSecond;
    struct tm tm;
    localtime_r(&tt, &tm);
    ts->tv_sec = mktime(&tm) + tm.tm_gmtoff - 3600*(tm.tm_isdst > 0?1:0);
    ts->tv_nsec = remainder_nanoseconds();
    return ts;
  }

};
    
class Duration {
private:
  friend class Time;
  long long t;
  
  Duration(long long n): t(n) {}

public:

  static Duration seconds(int sec)
  {
    Duration d;
    d.t = sec * Time::TicksPerSecond;
    return d;
  }

  Duration():t(0) {}
  Duration(const Time &t1, const Time &t2) {
    t = t1.t-t2.t;
  }

  ~Duration() {}

  bool operator < (int num) const {
    return (t < num);
  }

  bool operator < (const Duration &d) const {
    return (t < d.t * Time::TicksPerSecond);
  }

  bool operator ==  (int num) const {
    return (t == num * Time::TicksPerSecond);
  }

  bool operator == (const Duration &d) const {
    return (t == d.t);
  }

  COMPARISON_OPERATORS2(Duration, int)
  COMPARISON_OPERATORS(Duration)

  Duration &operator +=(Duration d) {
    t += d.t;
    return *this;
  }

  long long seconds() const {
    return t/Time::TicksPerSecond;
  }

  long long remainder_nanoseconds() const {
    return (1000000000/Time::TicksPerSecond)*(t % Time::TicksPerSecond);
  }

  friend std::ostream& operator << (std::ostream &o, const Duration &d);
	
};

static inline Duration operator -(const Time &t1, const Time &t2) {
  return Duration(t1, t2);
}

inline Time Time::operator + (const Duration &d) const
{
  Time t2;
  t2.t = t + d.t;
  return t2;
}

inline Time Time::operator - (const Duration &d) const
{
  Time t2;
  t2.t = t - d.t;
  return t2;
}

inline Duration Time::elapsedTime() const
{
  return Time::now() - (*this);
}

inline std::ostream& operator << (std::ostream &o, const Duration &d)
{
  char buff[30];
  snprintf(buff, sizeof(buff), "%lld.%06ld", d.seconds(), (long)d.remainder_nanoseconds());
  o << buff;
  return o;
}


}} //namespace
#endif // TEXTUS_BASE_TIME_H_
