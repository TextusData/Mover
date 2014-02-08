/* CacheTest.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
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
