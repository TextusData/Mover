/* DBStatementTest.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Test the DBInstance class and make a connection.
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
#include "textus/db/DBStatement.h"
#include "textus/db/DBHandle.h"
#include "textus/db/ColTypeUUID.h"

using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

#define MAX_ROWS 1000

namespace textus { namespace db {

class TestDB: public DBType {
  BEGIN_DB_DECL(TestDB);
  DECL_DB_ENTRY(TestDB, string, DBString<64>, typname, 0);
  DECL_DB_ENTRY(TestDB, int, DBInt, typlen, 0);
  END_DB_DECL(TestDB);

public:
  TestDB() {}
  virtual ~TestDB() {}

  string table() { return "pg_type"; }

};

STATIC_DB_DECL(TestDB);

class DBStatementTest: public UnitTest {
private:

public:
  DBStatementTest();
  ~DBStatementTest();

  void run_tests();
};

DBStatementTest::DBStatementTest(): UnitTest(__FILE__)
{
}

DBStatementTest::~DBStatementTest()
{
}

void DBStatementTest::run_tests() {
  AUTODEREF(DBInstance *, instance);
  fprintf(stderr, "Creating Test Instance...");
  instance = new DBInstance();
  assert(instance!= NULL);
  instance->setServer(getDSN());
  instance->setUser("unittest");
  fprintf(stderr, "Connecting...");
  assert (instance->connect() == 0);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Creating Statement...");
  AUTODEREF(DBStatement *, stmt);
  assert(instance->getHandle() != NULL);

  fprintf(stderr, "Allocating memory for result set...");
  TestDB *tdb=NULL;
  tdb = TestDB::allocResultSet(MAX_ROWS);

  assert(tdb != NULL);
  fprintf(stderr, "ok\n");

  fprintf(stderr, "executing query....");
  int res = TestDB::query("select typname, typlen from pg_type", MAX_ROWS, tdb, instance->getHandle());
  assert (res > 0);
  fprintf (stderr, "ok\n");

  /*for (int i = 0; i < res; i++) {
    fprintf (stderr, "%s:%d\n", tdb[i].typname().c_str(), tdb[i].typlen());
    } */

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new DBStatementTest();
}
