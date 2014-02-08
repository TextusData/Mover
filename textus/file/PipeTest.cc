/* PipeTest.cc -*- c++ -*-
 * Copyright (c) 2010
 *
 * Test the Pipe class.
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
#include "textus/file/Pipe.h"

using namespace textus::testing;
using namespace textus::file;

namespace textus { namespace file {

class PipeTest: public UnitTest {
public:
  PipeTest();
  ~PipeTest();

  void run_tests();
  
};

PipeTest::PipeTest(): UnitTest(__FILE__)
{
}

PipeTest::~PipeTest()
{
}

void PipeTest::run_tests() {
  string test("test");
  fprintf(stderr, "Testing pipes...");
  AutoDeref<Pipe> p;
  p = new Pipe();
  p->deref();
  p->writer()->write(test);
  string r = p->reader()->read(20);
  assert (r == test);
  fprintf(stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new PipeTest();
}
