/* TimerTest.cc -*- c++ -*-
 * Copyright (c) 2009, 2013
 *
 * Test the timer class.
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

#include <unistd.h>

#include "textus/testing/UnitTest.h"
#include "textus/event/Timer.h"
#include "textus/event/ConditionTimer.h"

using namespace textus::testing;
using namespace textus::event;

namespace textus { namespace event {

using namespace textus::base;

class TimerTest: public UnitTest {
    public:
    TimerTest();
    ~TimerTest();

    void run_tests();

};

TimerTest::TimerTest(): UnitTest(__FILE__)
{
}

TimerTest::~TimerTest()
{
}

void TimerTest::run_tests()
{
  fprintf (stderr, "Testing single ConditionTimer...");
  ConditionTimer *ct = new ConditionTimer();
  assert(ct != NULL);
  ct->timerStart(Duration::seconds(2));
  alarm(30000);
  ct->wait();
  fprintf (stderr, "ok\n");
  delete ct;

}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new TimerTest();
}
