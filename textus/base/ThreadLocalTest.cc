/* ThreadLocalTest.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Test the ThreadLocal class.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/base/ThreadLocal.h"
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

class ThreadLocalTest: public UnitTest {
private:
  ThreadLocal<int> tli;
  ThreadLocal<Counter *> tlc;

public:
  ThreadLocalTest();
  ~ThreadLocalTest();

  void run_tests();
};

ThreadLocalTest::ThreadLocalTest(): UnitTest(__FILE__)
{
}

ThreadLocalTest::~ThreadLocalTest()
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
  ThreadLocal<Counter *> *tlc = dynamic_cast<ThreadLocal<Counter *> * >(b);
  assert(tlc != NULL);
  assert((*tlc) == NULL);
  (*tlc) = new Counter();
  assert (*tlc != NULL);
  assert ((*tlc)->next() == 0);
  assert ((*tlc)->next() == 1);
  assert ((*tlc)->next() == 2);
  return NULL;
}

void ThreadLocalTest::run_tests() {
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
  th->join();
  th->deref();
  assert(tlc->next() == 2);
  fprintf (stderr, "ok\n");
  
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ThreadLocalTest();
}
