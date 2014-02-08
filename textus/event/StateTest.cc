/* StateTest.cc -*- c++ -*-
 * Copyright (c) 2010
 *
 * Test the State class.
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
#include "textus/event/State.h"

using namespace textus::testing;
using namespace textus::event;

namespace textus { namespace event {

using namespace textus::base;

#define TEST_MESSAGE1_TYPE MESSAGE_MAKE_TYPE(TEST_MESSAGE_CATEGORY, 1, 1, 1)
#define TEST_MESSAGE2_TYPE MESSAGE_MAKE_TYPE(TEST_MESSAGE_CATEGORY, 1, 1, 2)
#define TEST_MESSAGE3_TYPE MESSAGE_MAKE_TYPE(TEST_MESSAGE_CATEGORY, 1, 1, 3)
#define TEST_MESSAGE4_TYPE MESSAGE_MAKE_TYPE(TEST_MESSAGE_CATEGORY, 1, 1, 4)

class TestStateMachine: public State {
public:
  enum {
    State1 = State::LastReservedState + 1,
    State2,
    State3
  };

private:
  int testPrivateData;

protected:
  
  int transition1(Message *m) {
    fprintf (stderr, "%d, ", testPrivateData);
    testPrivateData = 1;
    assert(state() == StartState);
    return State1;
  }

  int transition2(Message *m) {
    fprintf (stderr, "%d, ", testPrivateData);
    testPrivateData = 2;
    assert(state() == State1);
    return State2;
  }

  int transition3(Message *m) {
    fprintf (stderr, "%d, ", testPrivateData);
    testPrivateData = 3;
    assert(state() == State2);
    return State3;
  }

  int done(Message *m) {
    fprintf (stderr, "%d ", testPrivateData);
    testPrivateData = 4;
    assert(state() == State3);
    return EndState;
  }

public:
  TestStateMachine(): testPrivateData(0) {
    addTransition(StartState, TEST_MESSAGE1_TYPE, static_cast<TransitionMember>(&TestStateMachine::transition1));
    addTransition(State1, TEST_MESSAGE2_TYPE, static_cast<TransitionMember>(&TestStateMachine::transition2));
    addTransition(State2, TEST_MESSAGE3_TYPE, static_cast<TransitionMember>(&TestStateMachine::transition3));
    addTransition(State3, TEST_MESSAGE4_TYPE, static_cast<TransitionMember>(&TestStateMachine::done));
  }

  virtual ~TestStateMachine() {}
};

class StateTest: public UnitTest {
    public:
    StateTest();
    ~StateTest();

    void run_tests();

};

StateTest::StateTest(): UnitTest(__FILE__)
{
}

StateTest::~StateTest()
{
}

void StateTest::run_tests()
{
  fprintf(stderr, "Testing State Machine...");
  AUTODEREF(TestStateMachine *, tsm);
  tsm = new TestStateMachine();
  assert (tsm != NULL);
  AUTODEREF(Message *, m);
  m = new Message();
  assert (m != NULL);
  m->setType(TEST_MESSAGE1_TYPE);
  assert(tsm->state() == State::StartState);
  tsm->processMessage(m);
  assert(tsm->state() == TestStateMachine::State1);
  m->setType(TEST_MESSAGE2_TYPE);
  tsm->processMessage(m);
  assert(tsm->state() == TestStateMachine::State2);
  m->setType(TEST_MESSAGE3_TYPE);
  tsm->processMessage(m);
  assert(tsm->state() == TestStateMachine::State3);
  m->setType(TEST_MESSAGE4_TYPE);
  tsm->processMessage(m);
  assert(tsm->state() == State::EndState);
  fprintf(stderr, "ok\n");
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new StateTest();
}
