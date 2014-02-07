/* BoolTest.cc -*- c++ -*-
 * Copyright (c) 2009
 *
 * Test the Bool class.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/logic/Bool.h"

using namespace textus::testing;
using namespace textus::base::logic;

namespace textus { namespace base { namespace logic {

class BoolTest: public UnitTest {
private:

public:
  BoolTest();
  ~BoolTest();

  void run_tests();
};

BoolTest::BoolTest(): UnitTest(__FILE__)
{
}

BoolTest::~BoolTest()
{
}

void BoolTest::run_tests() {
  fprintf (stderr, "Testing true ...");
  Bool t(true);

  assert(t);
  fprintf (stderr, "ok\n");

  fprintf (stderr, "Testing false...");
  Bool f(false);
  assert(!f);
  fprintf (stderr, "ok\n");

}

}}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new BoolTest();
}

