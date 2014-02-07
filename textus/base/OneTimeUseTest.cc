/* OneTimeUseTest.cc -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 *
 * Test the one time use integers.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/OneTimeUse.h"

using namespace textus::testing;
using namespace textus::base;

namespace textus { namespace base { 

class OneTimeUseTest: public UnitTest {
public:
  OneTimeUseTest(): UnitTest(__FILE__) {}
  virtual ~OneTimeUseTest() {}
  void run_tests();
};

static int used[10000];

typedef OneTimeUse<int> OTUInt;

static Base *useThread(Base *b) 
{
  OTUInt *otu = dynamic_cast<OTUInt *>(b);
  assert(otu != NULL);
  unsigned o;
  {
    Synchronized(otu);
  }

  for (o = (*otu); o < ARRAY_SIZE(used); o = (*otu)) {
    assert(used[o] == 0);
    used[o]++;
    assert(used[o] == 1);
  }
  return NULL;
}

void OneTimeUseTest::run_tests() {
  Thread *threads[17];
  AutoDeref<OneTimeUse<int> > otu;

  fprintf(stderr, "Creating otu object...");
  otu = new OneTimeUse<int>(0);
  if (otu != NULL) {
    otu->deref();
  }
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Getting 10 OTU ints....");
  for (int i = 0; i < 10; ++i) {
    int j = (*otu);
    assert(used[j] == 0);
    used[j]++;
    assert(used[j] == 1);
  }
  fprintf(stderr, "ok\n");

  fprintf(stderr, "Grabbing a bunch of one time use integers...");

  memset(used, 0, sizeof(used));

  otu = new OneTimeUse<int>(0);
  if (otu != NULL) {
    otu->deref();
  }

  for (unsigned i = 0; i < ARRAY_SIZE(threads); i++) {
    Synchronized(otu);
    threads[i] = new Thread(useThread, otu);
    assert(threads[i] != NULL);
  }
  for (unsigned i = 0; i < ARRAY_SIZE(threads); i++) {
    threads[i]->join();
    threads[i]->deref();
    threads[i] = NULL;
  }
  fprintf (stderr, "ok\n");
  fprintf(stderr, "Making sure we got all of the integers...");
  for (unsigned i = 0; i < ARRAY_SIZE(used); ++i) {
    assert(used[i] <= 1);
  }

  for (unsigned i = 0; i < ARRAY_SIZE(used); ++i) {
    assert(used[i] == 1);
  }

  fprintf(stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new OneTimeUseTest();
}

