/* TimerEventTest.cc -*- c++ -*-
 * Copyright (c) 2009
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
#include "textus/event/ConditionTimerEvent.h"
#include "textus/event/EventQueue.h"
#include "textus/base/Thread.h"

using namespace textus::testing;
using namespace textus::event;

namespace textus { namespace event {

using namespace textus::base;

class TimerEventTest: public UnitTest {
    public:
    TimerEventTest();
    ~TimerEventTest();

    void run_tests();

};

static Base *startWorkerThread(Base *b)
{
  EventQueue *eq = dynamic_cast<EventQueue *>(b);
  eq->threadLoop();
  return NULL;
}

TimerEventTest::TimerEventTest():UnitTest(__FILE__)
{
}

TimerEventTest::~TimerEventTest()
{
}

void TimerEventTest::run_tests()
{
  fprintf (stderr, "Testing single ConditionTimerEvent...");

  ConditionTimerEvent *ct = new ConditionTimerEvent();
  assert(ct != NULL);

  EventQueue *eq = new EventQueue();
  assert(eq != NULL);

  Thread *workerThread = new Thread(startWorkerThread, eq);
  assert(workerThread != NULL);

  EventQueue::setPreferredQueue(eq);

  ct->timerStart(Duration::seconds(2));

  alarm(5000);
  ct->wait();
  alarm(0);

  fprintf (stderr, "ok\n");

  eq->threadExit();

  workerThread->join();

  workerThread->deref();

  ct->deref();
  eq->deref();
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new TimerEventTest();
}
