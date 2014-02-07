/* DBTypeTest.cc -*- c++ -*-
 * Copyright (c) 2011-2012 Ross Biro
 *
 * Test the DBType class.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/db/DBType.h"
#include "textus/db/ColTypeUUID.h"


using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

namespace textus { namespace db {

class TestDB: public DBType {
  BEGIN_DB_DECL(TestDB);
  DECL_DB_ENTRY(TestDB, long, Number<10>, id, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(TestDB, string, DBString<100>, name, 0);
  DECL_DB_ENTRY(TestDB, string, DBString<100>, address, 0);
  END_DB_DECL(TestDB);

public:
  TestDB() {}
  virtual ~TestDB() {}

  string table() { return "test"; }
  
};

STATIC_DB_DECL(TestDB);

class DBTypeTest: public UnitTest {
private:

public:
  DBTypeTest();
  ~DBTypeTest();

  void run_tests();
};

DBTypeTest::DBTypeTest(): UnitTest(__FILE__)
{
}

DBTypeTest::~DBTypeTest()
{
}

void DBTypeTest::run_tests() {
  AUTODEREF(TestDB *, tdb);
  fprintf(stderr, "Creating TestDB...");
  tdb = new TestDB();
  assert(tdb != NULL);
  fprintf(stderr, "ok\n");

  fprintf(stderr, "Generating query SQL...");
  string sql = tdb->sql(dbQuery);
  assert (sql.compare("select id, name, address from test where id = ?;") == 0);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Generating insert SQL...");
  tdb->set_name(string("John Doe"));
  tdb->set_address(string("1600 Pennsylvania Ave"));
  sql = tdb->sql(dbInsert);
  assert (sql.compare("insert into test ( id, name, address ) values ( ?, ?, ?);") == 0);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Generating update SQL...");
  sql = tdb->sql(dbUpdate);
  assert (sql.compare("update test set id = ?, name = ?, address = ? where id = ?;") == 0);
  fprintf (stderr, "ok\n");

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new DBTypeTest();
}
