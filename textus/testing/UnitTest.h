/* UnitTest.h -*- C++ -*-
 * Copyright (C) 2009-2010, 2013 Ross Biro
 *
 * A class to help test.
 *
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

#ifndef TEXTUS_TESTING_UNIT_TEST_H_
#define TEXTUS_TESTING_UNIT_TEST_H_

#define TESTING 1
#include <stdio.h>

#include "textus/base/Base.h"
#include "textus/file/Directory.h"
#include "textus/base/ThreadLocal.h"

#include <list>

namespace textus { namespace testing {

using namespace std;

class UnitTest: virtual public textus::base::Base {
private:
  static textus::base::ThreadLocal<UnitTest *> current_test_object;
  static std::string getCurrentDSN(); // should only be used to test low level DB access code.
  static std::string getCurrentPoolDSN();
  int failed_tests;
  textus::base::AutoDeref<textus::file::Directory> data_dir;
  textus::base::AutoDeref<textus::file::Directory> tmp_dir;
  list<std::string> tmp_dbs; // name of tmp database(s) for test use.
  const char *srcFile;

  string createNewDB(string script, int count = 1);

protected:
  virtual const char *dbScript() { return NULL; }
  virtual string getDB() { if (!tmp_dbs.empty()) return tmp_dbs.front(); return ""; }
  virtual void addDB(string db) { tmp_dbs.push_back(db); }

public:
  explicit UnitTest(const char *src):failed_tests(0), srcFile(src) {
    current_test_object = this;
  }
  ~UnitTest() {}

  void test(bool test) {
    if (!test) {
      fprintf (stderr, "Test Failed\n");
      failed_tests++;
    }
  }

  void setup();
  void cleanup();
  virtual void prepare_tests();
  virtual void run_tests();
  virtual void cleanup_tests();

  virtual textus::file::Directory *getDataDir();
  virtual textus::file::Directory *getTmpDir();
  virtual textus::file::FileHandle *getTmpFile();
  virtual std::string getDSN();
  virtual std::string getPoolDSN(int count=2);

  const char *getSrcFile() const {
    return srcFile;
  }

  int getFailedTests() { return failed_tests; }

  static string poolDSN();

};

}} //namespace

textus::testing::UnitTest *createTestObject(int argc, const char *argv[]);


#endif // TEXTUS_TESTING_UNIT_TEST_H_
