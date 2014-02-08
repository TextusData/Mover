/* UnitTest.cc -*- C++ -*-
 * Copyright (C) 2009-2014 Ross Biro
 *
 * A class to help test.
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

#include "textus/base/init/Init.h"
#include "textus/system/Environment.h"
#include "textus/file/SystemFile.h"
#include "textus/file/Directory.h"
#include "textus/file/Shell.h"
#include "textus/db/DBUtils.h"
#include "textus/base/UUID.h"
#include "textus/db/DBPool.h"

#include <stdio.h>
#include <stdlib.h>

#include <string>

namespace textus { namespace testing {

using namespace std;

textus::base::ThreadLocal<UnitTest *> UnitTest::current_test_object;

void UnitTest::prepare_tests()
{
}

void UnitTest::run_tests()
{
  /* No Tests. This is an error. */
  fprintf (stderr, "run_tests not overriden.\n");
  test(false);
}

void UnitTest::cleanup_tests()
{
    
}

void  UnitTest::cleanup() {
  int ret=0;
  Synchronized(this);
  if (tmp_dir) {
    tmp_dir->recursiveUnlink();
    tmp_dir = NULL;
  }

  textus::db::DBPool::cleanup();

  for (list<std::string>::iterator i = tmp_dbs.begin(); i != tmp_dbs.end(); ++i) {
    AUTODEREF(textus::db::DBUtils *, dbu);
    dbu = new textus::db::DBUtils();
    HRC(dbu->init());
    dbu->setUser("unittest");
    dbu->setDSN("Test");
    HRC (dbu->connect());
    HRC (dbu->dropDB(*i));
  }
 error_out:
  if (ret != 0) {
    failed_tests++;
  }
  return;
}



void UnitTest::setup() {
  textus::db::DBPool::default_dsn_func = getCurrentPoolDSN;
}

textus::file::Directory *UnitTest::getDataDir() {
  Synchronized(this);
  if (data_dir) {
    return data_dir;
  }

  // srcfile will look something like ./textus or textus or .. we
  // insert test_data/ just before the textus.
  string dp(srcFile); 
  
  // Could shell out to git and find the correct root directory. For
  // now we assume we are running from the build directory.
  AUTODEREF(textus::file::Shell *, shell);
  shell = new textus::file::Shell();
  shell->setCommand("git rev-parse --show-toplevel");
  shell->go();
  shell->waitForCompletion();
  string output = shell->getOutput();
  size_t end = output.find_first_of("\n\r", 0);
  if (end != string::npos) {
    output = output.substr(0, end);
  }
  string::size_type s = dp.find("textus");
  if (s == string::npos) {
    LOG(ERROR) << "Unable to locate base directory in " << dp << "\n";
    die();
  }
  dp = dp.substr(s);
  dp = dp.insert(0, "/test_data/");
  // XXXXXX FIXME: valgrind thinks this is a leak. Supression: UnitTest test data
  dp = dp.insert(0, output);
  
  data_dir = textus::file::Directory::lookup(dp);

  if (data_dir) {
    data_dir->deref();
  }

  // Modify app dir to point to data dir as well.
  textus::system::Environment::modifySystemEnvironment(string("APP_PATH"), dp);
  return data_dir;
}

DEFINE_STRING_ARG(system_tmp_dir, "", "tmp-dir", "Base directory for temp files.  Defaults to $TMPDIR if set, or /tmp if not.");

textus::file::Directory *UnitTest::getTmpDir() {
  Synchronized(this);
  if (tmp_dir) {
    return tmp_dir;
  }

  string tmp;
  if (system_tmp_dir != "") {
    tmp = system_tmp_dir;
  } else {
    tmp = textus::system::Environment::systemEnvironment().getVariable("TMPDIR");
    if (tmp == "") {
      tmp = textus::system::Environment::systemEnvironment().getVariable("TMP");
    }
    if (tmp == "") {
      tmp = string("/tmp");
    }
  }

  // Cheat and reexpand environment variables.
  tmp = textus::system::Environment::systemEnvironment().expandStrings(tmp);

  // Add the unique part to this name.
  tmp = tmp + "/" + textus::file::SystemFile::generateUniqueName();

  tmp_dir = textus::file::Directory::lookup(tmp);
  if (tmp_dir) {
    if (!tmp_dir->createPath()) {
      tmp_dir->deref();
      tmp_dir = NULL;
      LOG(ERROR) << "Unable to create directory: " << tmp << "\n";
      die();
    }
    tmp_dir->deref();
  }
  textus::system::Environment::mutableSystemEnvironment().setVariable("TMPDIR", tmp);

  return tmp_dir;
}

textus::file::FileHandle *UnitTest::getTmpFile() {
  textus::file::Directory *tmpdir = getTmpDir();
  string tmp = textus::file::SystemFile::generateUniqueName();
  return tmpdir->openFile(tmp, O_RDWR|O_CREAT);
}

string UnitTest::createNewDB(string script, int count) {
  int ret = 0;
  AUTODEREF(textus::db::DBUtils *, dbu);
  textus::base::UUID uuid;
  string db;
  string dsn;
  dbu = new textus::db::DBUtils();
  if (dbu->init()) {
    return "";
  }
  dbu->setUser("unittest");
  dbu->setDSN("Test");
  HRC(dbu->connect());
  for (int i = 0; i < count; ++i) {
    uuid.generate();
    db = string("tst") + uuid.toString();
    while (1) {
      size_t s = db.find("-");
      if (s == string::npos) {
	break;
      }
      db.replace(s, 1, 1, '_');
    }
    HRC(dbu->createDB(db));
    if (i == 0) {
      textus::system::Environment::mutableSystemEnvironment().setVariable("TEST_DSN", dsn);
    }
    char buff[30];
    snprintf (buff, sizeof(buff), "TEST_DSN%d", i);
    dsn = string("Driver=/usr/local/lib/psqlodbcw.so;Server=localhost;Port=5432;Database=") + db + ";";
    addDB(db);
    textus::system::Environment::mutableSystemEnvironment().setVariable(buff, dsn);
  }
  dbu->close();
  
  for (int i = 0; i < count; ++i) {
    dbu->init();
    char buff[30];
    snprintf (buff, sizeof(buff), "TEST_DSN%d", i);
    dsn = textus::system::Environment::systemEnvironment().getVariable(buff);
    dbu->setDSN(dsn);
    dbu->setUser("unittest");
    HRC(dbu->connect());
    HRC(dbu->executeScript(script, &textus::system::Environment::systemEnvironment()));
  }
    
  dbu->close();
  dsn = string("Driver=/usr/local/lib/psqlodbcw.so;Server=localhost;Port=5432;Database=") + db + ";";
  return dsn;
 error_out:
  return "";
}

string UnitTest::getCurrentPoolDSN() {
  return current_test_object->getPoolDSN();
}

string UnitTest::getPoolDSN(int count) {
  // First make sure that ODBC DSNs are available.
  string s = textus::system::Environment::systemEnvironment().expandStrings("$(APP_PATH)/db/odbc.ini");
  textus::system::Environment::exportSystemEnvironment(string("ODBCINI"), s);

  const char *scr = dbScript();
  if (scr != NULL) {
    string script = textus::system::Environment::systemEnvironment().expandStrings(scr);
    return createNewDB(script, count);
  }
         
  return string("Test");
}

// Needs to create a tmp database and point a DSN to it.
string UnitTest::getDSN() 
{
  // First make sure that ODBC DSNs are available.
  string s = textus::system::Environment::systemEnvironment().expandStrings("$(APP_PATH)/db/odbc.ini");
  textus::system::Environment::exportSystemEnvironment(string("ODBCINI"), s);

  const char *scr = dbScript();
  if (scr != NULL) {
    string script = textus::system::Environment::systemEnvironment().expandStrings(scr);
    return createNewDB(script);
  }
         
  return string("Test");
}

}} //namespace

using namespace textus::testing;

namespace textus { namespace base {
// define this strongly here.  The one in Base.cc is weak, so this should
// override it.
bool testing = true;
}}

int main(int argc, const char *argv[], const char *envp[])
{
  int ret;

  // Set this first so it can be overridden during TextusInit
  textus::base::init::ArgumentAppender::setArg("--log-to-stderr");

  textus::base::init::TextusInit(argc, argv, envp);

  AUTODEREF(UnitTest *, testObj);
  testObj = createTestObject(argc, argv);
  if (!testObj) {
    fprintf (stderr, "Unable to create test object.\n");
    exit (1);
  }

  testObj->getDataDir(); // this has side effects that mess with the environment

  fprintf (stderr, "Running tests from %s\n", testObj->getSrcFile());

  testObj->setup();
  testObj->prepare_tests();
  testObj->run_tests();
  testObj->cleanup_tests();
  testObj->cleanup();

  ret = testObj->getFailedTests();

  textus::base::init::TextusShutdown();

  if (ret) {
    fprintf(stderr, "test exiting with return value =%d\n", ret);
  }

  return ret;  
}



