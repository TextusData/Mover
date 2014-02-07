/* DateTime.h -*- c++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 * Class Represents a date and time.
 *
 */

#ifndef TEXTUS_TIME_DATETIME_H_
#define TEXTUS_TIME_DATETIME_H_

namespace textus { namespace time {

/*
 * We use picoseconds as a base because it's unlikely we'll
 * ever need to worry about times less than 1 nanosecond.
 * With nanoseconds, we can represent about 10^9 seconds from
 * the base with full resolution, which is about 38 years.
 *
 * We will try to use significant figure like accuracy on the resolution
 * when converting between base years and doing arithmetic.
 *
 */

class DateTime /* lightweight class, no base. */ {
  static int base_year; /* We start measuring from January 1, 00:00:00.0000000 on this year. */
  static const int seconds_per_day = 24*60*60;
  static const int seconds_per_year = 365*seconds_per_day;
  static const int seconds_per_leap_year = seconds_per_year + seconds_per_day;

  int64_t time; // number of resolution nanoseconds from base year..
  uint64_t  resolution; // number of nanoseconds in each tick of time above.
};



}} //namespace

#endif //TEXTUS_TIME_DATETIME_H_
