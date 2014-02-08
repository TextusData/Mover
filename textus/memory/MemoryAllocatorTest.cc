/* MemoryAllocatorTest.cc -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 *
 * Test the MemoryAllocator class.
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
#include "textus/memory/MemoryAllocator.h"

using namespace textus::testing;
using namespace textus::base;
using namespace textus::memory;

namespace textus { namespace memory {

class MemoryAllocatorTest: public UnitTest {
private:
  MemoryAllocator b;

public:
  MemoryAllocatorTest();
  ~MemoryAllocatorTest();

  void run_tests();
};

MemoryAllocatorTest::MemoryAllocatorTest(): UnitTest(__FILE__)
{
}

MemoryAllocatorTest::~MemoryAllocatorTest()
{
}


void MemoryAllocatorTest::run_tests() {
  AUTODEREF(MemoryAllocator *, ma);
  fprintf (stderr, "creating memory allocator...");
  ma = new MemoryAllocator();
  fprintf (stderr, "ok\n");

  for (int i = 1; i < 1024; i = i >> 1) {
    fprintf( stderr, "testing allocator size %d...", i);
    void *v = ma->allocate(i);
    fprintf (stderr, "ok\n");
    fprintf (stderr, "testing freeing size %d...", i);
    ma->free(v);
    fprintf (stderr, "ok\n");
  }

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new MemoryAllocatorTest();
}
