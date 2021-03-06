/* Time.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * A class that represents the time.
 */

/*
 *   This program is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, version 3 of the
 *   License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see
 *   <http://www.gnu.org/licenses/>.
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "textus/base/Time.h"


namespace textus { namespace base {

Time Time::now() 
{
  struct tm tm;
  time_t n;
  time(&n);
  localtime_r(&n, &tm);
  Time t(&tm);
  return t;
}

}} //namespace
