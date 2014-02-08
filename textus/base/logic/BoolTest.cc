/* BoolTest.cc -*- c++ -*-
 * Copyright (c) 2009
 *
 * Test the Bool class.
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

