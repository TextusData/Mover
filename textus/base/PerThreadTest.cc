/* PerThreadTest.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Test the PerThread class.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/base/PerThread.h"
#include "textus/base/Thread.h"

using namespace textus::testing;
using namespace textus::base;

namespace textus { namespace base {

class Counter {
private:
  int c;

public:
  Counter():c(0) {}
  virtual ~Counter() {}
  int next() { return c++; }
};

class PerThreadTest: public UnitTest {
private:
  ThreadLocal<int> tli;
  PerThread<Counter *> tlc;

public:
  PerThreadTest();
  ~PerThreadTest();

  void run_tests();
};

PerThreadTest::PerThreadTest(): UnitTest(__FILE__)
{
}

PerThreadTest::~PerThreadTest()
{
}

static Base *otherThread(Base *b) {
  ThreadLocal<int> *tli = dynamic_cast<ThreadLocal<int> * >(b);
  assert(tli != NULL);
  assert (*tli != 1);
  *tli = 2;
  assert (*tli == 2);
  return NULL;
}

static Base *otherThreadObject(Base *b) {
  PerThread<Counter *> *tlc = dynamic_cast<PerThread<Counter *> * >(b);
  assert(tlc != NULL);
  assert(static_cast<Counter *>(*tlc) == NULL);
  Synchronized(tlc);
  (*tlc) = new Counter();
  assert ((*tlc) != NULL);
  assert ((*tlc)->next() == 0);
  assert ((*tlc)->next() == 1);
  assert ((*tlc)->next() == 2);
  tlc->wait();
  return NULL;
}

void PerThreadTest::run_tests() {
  fprintf (stderr, "Testing getting int from thread local variable...");
  tli = 1;
  assert (tli == 1);
  fprintf (stderr, "ok\n");
  fprintf (stderr, "Testing other thread...");
  Thread *th = new Thread(otherThread, static_cast<Base *>(&tli));
  assert(th != NULL);
  th->join();
  th->deref();
  th = NULL;
  assert (tli == 1);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Testing getting object from thread local variable...");
  tlc = new Counter();
  assert(tlc != NULL);
  assert(tlc->next() == 0);
  assert(tlc->next() == 1);
  fprintf(stderr, "ok\n");

  fprintf (stderr, "Testing getting object in other thread...");
  th = new Thread(otherThreadObject, static_cast<Base *>(&tlc));
  assert(th != NULL);
  fprintf (stderr, "ok\n");

  // make sure there is more than one now.
  fprintf (stderr, "Making sure we can access both objects...");
  int count = 0;
  {
    Synchronized(&tlc);
    tlc.wait();

    PerThread<Counter *>::iterator i;

    for (i = tlc.begin(); i != tlc.end(); ++i) {
      count++;
    }
  }
  assert(count == 2);
  fprintf(stderr, "ok\n");
  th->join();
  th->deref();
  assert(tlc->next() == 2);
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new PerThreadTest();
}
