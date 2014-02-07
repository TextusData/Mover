/* BaseTest.cc -*- c++ -*-
 * Copyright (c) 2009-2012 Ross Biro
 *
 * Test the base class.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/base/Thread.h"

using namespace textus::testing;
using namespace textus::base;

namespace textus { namespace base {

class BaseTest: public UnitTest {
private:
  Base b;

public:
  BaseTest();
  ~BaseTest();

  void run_tests();
};

BaseTest::BaseTest(): UnitTest(__FILE__)
{
}

BaseTest::~BaseTest()
{
}

static volatile int count = 0;
    
static Base *test_thread2(Base *b)
{
  for (int i = 0; i < 1000; i++)
    {
      Synchronized(b);
      int tmp_count = count;
      tmp_count++;
      struct timespec rqtp;
      rqtp.tv_nsec = 1;
      rqtp.tv_sec = 0;
      nanosleep(&rqtp, NULL);
      count = tmp_count;
    }
  return NULL;
}

void BaseTest::run_tests() {
  {
    fprintf (stderr, "Testing sync_count 1...");
    Synchronized(&b);
    assert(b.sync_count == 1);
    fprintf (stderr, "ok\n");

    fprintf (stderr, "Testing sync_count 2...");
    Synchronized(&b);
    assert(b.sync_count == 2);
    fprintf (stderr, "ok\n");

    fprintf (stderr, "Testing sync_thread_id...");
    assert(b.sync_thread_id == ThreadID::self());
    fprintf (stderr, "ok\n");
  }

  fprintf (stderr, "Testing sync...");
  Thread *t;
  {
    Synchronized(&b);
    t = new Thread(test_thread2, &b);
    assert(t);
    assert(count == 0);
  }

  while (count == 0) {
    struct timespec rqtp;
    rqtp.tv_nsec = 1;
    rqtp.tv_sec = 0;
    nanosleep(&rqtp, NULL);
  }

  for (int i = 0; i < 1000; i++) {
    Synchronized(&b);
    int tmp_count = count;
    assert(b.sync_thread_id == ThreadID::self());
    struct timespec rqtp;
    rqtp.tv_nsec = 1;
    rqtp.tv_sec = 0;
    nanosleep(&rqtp, NULL);
    tmp_count++;
    count = tmp_count;
  }

  for (int i = 0; i < 1000; i++) {
    if (count == 2000) break;
    struct timespec rqtp;
    rqtp.tv_nsec = 1;
    rqtp.tv_sec = 0;
    nanosleep(&rqtp, NULL);
  }

  Synchronized(&b);
  assert(count == 2000);

  t->join();
  assert(t->deref());

  fprintf (stderr, "ok\n");

  fprintf (stderr, "Testing refernence counting...");
  // One for b and one for our thread object.
  int ic = Base::instance_count;
  Base *b2 = new Base;

  assert(b2->ref_count == 1);
  assert(Base::instance_count == ic + 1);

  b2->ref();
  assert(b2->ref_count == 2);

  assert(b2->deref() == 0);
  assert(b2->ref_count == 1);

  assert(b2->deref() == 1);

  //b2 should have been deleted.
  assert(Base::instance_count == ic);

  fprintf (stderr, "ok\n");

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new BaseTest();
}
