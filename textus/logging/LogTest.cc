/* LogTest.cc -*- c++ -*-
 * Copyright (c) 2009, 2013
 *
 * Test the base class.
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

#include "textus/testing/UnitTest.h"
#include "textus/logging/Log.h"

using namespace textus::testing;
using namespace textus::logging;

namespace textus { namespace logging {

class LogTest: public UnitTest {
public:
  LogTest();
  ~LogTest();

  void run_tests();

};

LogTest::LogTest():UnitTest(__FILE__)
{
}

LogTest::~LogTest()
{
}

void LogTest::run_tests()
{
  // do this a bunch of times.  Valgrind detects a memory leak
  // and I can't tell if it's fake or not.  If it stays constant,
  // then it's fake.  If it increases as we do more loops, then it's
  // real.
  for (int i = 0; i < 100; ++i) {
    fprintf (stderr, "Testing logging...");
    LOG(ERROR) << "This ia a test, it is only a test.\n";
    fprintf (stderr, "ok\n");
    fprintf(stderr, "Testing logging call stack...");
    LOG_CALL_STACK(ERROR);
    fprintf(stderr, "ok\n");
  }
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new LogTest();
}

