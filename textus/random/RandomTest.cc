/* RandomTest.cc -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
 *
 * Test the Random class.
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
#include "textus/random/Random.h"
#include "textus/random/PRNG.h"

using namespace textus::testing;
using namespace textus::random;

namespace textus { namespace random {

class RandomTest: public UnitTest {
public:
  RandomTest();
  ~RandomTest();

  void run_tests();
};

RandomTest::RandomTest(): UnitTest(__FILE__)
{
}

RandomTest::~RandomTest()
{
}

void RandomTest::run_tests() {
  fprintf (stderr, "Attemting to get globalPRNG()....");
  PRNG *prng = PRNG::globalPRNG();
  assert(prng != NULL);
  fprintf(stderr, "ok\n");

  fprintf (stderr, "Attempting to seed PRNG....");
  prng->seed();
  fprintf (stderr, "ok\n");

  fprintf (stderr, "Attempting to generate random numbers....");
  for (int i = 1; i < 1000; ++i) {
    int r = prng->rand(i);
    assert (r >= 0 && r < i);
  }
  fprintf (stderr, "ok\n");

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new RandomTest();
}
