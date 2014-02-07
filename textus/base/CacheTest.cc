/* CacheTest.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Test the base class.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/base/ReferenceValueCache.h"

using namespace textus::testing;
using namespace textus::base;

namespace textus { namespace base {

class CacheObj: public Base {
public:
  static int count;
  CacheObj() {
    count++;
  }
  virtual ~CacheObj() {
    count--;
  }
};

int CacheObj::count = 0;

class CacheTest: public UnitTest {

public:
  CacheTest();
  ~CacheTest();

  void run_tests();
};

CacheTest::CacheTest():UnitTest(__FILE__)
{
}

CacheTest::~CacheTest()
{
}

// but the 20 explitily here so the tests will still work if someone
// changes the default.
typedef ReferenceValueCache<int, Base *, 20> rvc;

void CacheTest::run_tests() {
  int count;
  count = CacheObj::count;
  assert(count == 0);
  {
    fprintf (stderr, "Testing Creating Cache...");
    rvc c1;
    for (int i = 0; i < 50; i++) {
      AUTODEREF(CacheObj *, co);
      co = new CacheObj();
      c1[i] = co;
    }
    assert(CacheObj::count = count + 20);
    assert(c1.size() == 20);
    fprintf (stderr, "ok\n");
    fprintf (stderr, "Testing cache destruction...");
    }
  assert (CacheObj::count == count);
  fprintf (stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new CacheTest();
}
