/* ColTest.cc -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 *
 * Test the ColType classes.
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
#include "textus/db/ColType.h"

using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

namespace textus { namespace db {

class ColTest: public UnitTest {
private:

public:
  ColTest();
  ~ColTest();

  void run_tests();
};

ColTest::ColTest(): UnitTest(__FILE__)
{
}

ColTest::~ColTest()
{
}

void ColTest::run_tests() {
  fprintf (stderr, "Creating DBString....");
  AUTODEREF(DBString<100> *, ds);
  ds = new DBString<100>();
  assert(ds != NULL);
  fprintf (stderr, "ok\n");

  fprintf (stderr, "Creating Number....");
  typedef Number<100, 10> BigNum;

  AUTODEREF(BigNum *, no);
  no = new BigNum();
  assert(no != NULL);
  fprintf (stderr, "ok\n");

  typedef Number<100, 0> BigInt;
  fprintf (stderr, "Creating Integer....");

  AUTODEREF(BigInt *, ni);
  ni = new BigInt();
  assert(ni != NULL);
  fprintf (stderr, "ok\n");

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ColTest();
}
