/* DBManyManyTest.cc -*- c++ -*-
 * Copyright (c) 2012-2013 Ross Biro
 *
 * Test the DBManyMany template to hold two foreign keys and be able to
 * dereference them.
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
#include "textus/db/DBManyMany.h"
#include "textus/db/DBHandle.h"

using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

#define MAX_ROWS 1000

namespace textus { namespace db {

class TestDB2: public DBType {
  BEGIN_DB_DECL(TestDB2);
  DECL_DB_ENTRY(TestDB2, UUID, ColTypeUUID, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(TestDB2, string, DBString<100>, name, 0);
  END_DB_DECL(TestDB2);

public:
  TestDB2() {}
  virtual ~TestDB2() {}

  string table() { return "test_table_2"; }

};

class TestDB1: public DBManyMany<TestDB2, TestDB2> {
public:
  TestDB1() {}
  virtual ~TestDB1() {}
  TestDB2 *colA(DBHandle *h) { return get_column_key1()->refValue(h); }
  TestDB2 *colB(DBHandle *h) { return get_column_key2()->refValue(h); }
  string table() { return "test_table_1"; }

};

STATIC_DB_DECL(TestDB2);

class DBManyManyTest: public UnitTest {
private:
  const char *dbScript() { return "$(APP_PATH)/db/create_db_script"; }

public:
  DBManyManyTest();
  ~DBManyManyTest();

  void run_tests();
};

DBManyManyTest::DBManyManyTest(): UnitTest(__FILE__)
{
}

DBManyManyTest::~DBManyManyTest()
{
}

void DBManyManyTest::run_tests() {
  AUTODEREF(DBInstance *, instance);
  fprintf(stderr, "Creating Test Instance...");
  instance = new DBInstance();
  assert(instance!= NULL);
  instance->setServer(getDSN());
  instance->setUser("unittest");
  fprintf(stderr, "Connecting...");

  assert (instance->connect() == 0);
  assert(instance->getHandle() != NULL);
  fprintf (stderr, "ok\n");

  TestDB1 tdb;

  fprintf(stderr, "executing query....");
  int res = TestDB1::query("select * from test_table_1", 1, &tdb, instance->getHandle());
  assert (res == 1);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "resolving key1....");
  TestDB2 *tdb2 = tdb.get_column_key1()->refValue(instance->getHandle());
  assert(tdb2 != NULL);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "resolving key2....");
  tdb2 = tdb.get_column_key2()->refValue(instance->getHandle());
  assert(tdb2 != NULL);
  fprintf (stderr, "ok\n");

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new DBManyManyTest();
}
