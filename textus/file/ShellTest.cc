/* ShellTest.cc -*- c++ -*-
 * Copyright (c) 2010-2014
 *
 * Test the shell class.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "textus/testing/UnitTest.h"
#include "textus/file/Shell.h"
#include "textus/logging/Log.h"

using namespace textus::testing;
using namespace textus::file;

namespace textus { namespace file {

class ShellTest: public UnitTest {
public:
  ShellTest();
  ~ShellTest();

  void run_tests();
  
};

ShellTest::ShellTest(): UnitTest(__FILE__)
{
}

ShellTest::~ShellTest()
{
}

void ShellTest::run_tests() {
  string long_string;
  for (int i = 0; i < 4000; ++i) {
    long_string = long_string + "This is a test.  This is only a test.\n";
  }

  for (int i = 0; i < 20; ++i) {
    fprintf (stderr, "testing shell command...");
    AUTODEREF(Shell *, sh);
    sh = new Shell();
    assert(sh != NULL);
    sh->setCommand("echo this is a test");
    sh->go();
    sh->waitForCompletion();
    string err = sh->getErrorOutput();
    if (err.length() != 0) {
      fprintf (stderr, "err = %s", err.c_str());
      assert(err == "");
    }
    if (sh->exitStatus() != 0) {
      fprintf(stderr, "echo failed...."); // This happens on mac os-x.
    } else {
      string s = sh->getOutput();
      if (s != string("this is a test\n")) {
	LOG(ERROR) << s  << " != \"this is a test\" " << "\n";
	fprintf(stderr, "ERROR %s != this is a test\n", s.c_str());
	assert (0);
      }
    }
    fprintf(stderr, "ok\n");
    sh->deref();
    fprintf (stderr, "Testing shell buffered i/o...");
    sh = new Shell();
    assert (sh != NULL);
    sh->setCommand("cat");
    sh->setInput(long_string);
    sh->go();
    sh->waitForCompletion();
    assert(sh->exitStatus() == 0);
    string s = sh->getOutput();
    assert (s == long_string);
    fprintf (stderr, "ok\n");
  }
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ShellTest();
}
