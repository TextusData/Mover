/* ColTypeKeyTest.cc -*- c++ -*-
 * Copyright (c) 2012- 2013 Ross Biro
 *
 * Test the ColTypeKey column type.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/db/ColTypeKey.h"
#include "textus/db/ColTypeUUID.h"
#include "textus/db/DBHandle.h"
#include "textus/base/UUID.h"


using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

namespace textus { namespace db {

class TestDB: public DBType {
  BEGIN_DB_DECL(TestDB);
  DECL_DB_ENTRY(TestDB, UUID, ColTypeUUID, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(TestDB, Key *, ColTypeKey, key, 0);
  END_DB_DECL(TestDB);

public:
  TestDB() {}
  virtual ~TestDB() {}

  string table() { return "test_table"; }

};


STATIC_DB_DECL(TestDB);

class ColTypeKeyTest: public UnitTest {
private:
  const char *dbScript() { return "$(APP_PATH)/db/create_db_script"; }

public:
  ColTypeKeyTest();
  ~ColTypeKeyTest();

  void run_tests();
};

ColTypeKeyTest::ColTypeKeyTest(): UnitTest(__FILE__)
{
}

ColTypeKeyTest::~ColTypeKeyTest()
{
}

void ColTypeKeyTest::run_tests() {
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

  fprintf(stderr, "Adding row to test table....");
  TestDB tdb;
  AUTODEREF(Key *, key1);
  key1 = Key::createKey(RSA_PK_TYPE);
  tdb.set_key(key1);
  dynamic_cast<ColTypeUUID *>(tdb.get_column_pk())->generate();
  
  string sql = tdb.sql(dbInsert, "");
  
  AUTODEREF(DBStatement *, stmt);
  stmt = instance->getHandle()->prepare(sql);
  assert (stmt != NULL);
  assert(tdb.bindParams(stmt,dbQuery) == 0);
  assert(stmt->execute() == 0);
  fprintf (stderr, "ok\n");

  TestDB tdb2;

  fprintf(stderr, "executing query....");
  int res = TestDB::query("select * from test_table", 1, &tdb2, instance->getHandle());
  assert (res == 1);
  fprintf (stderr, "ok\n");

  fprintf (stderr, "Checking to see if we got a key back...");
  AUTODEREF(Key *, key2);
  key2 = tdb2.key();
  assert (key2 != NULL);
  assert (*key2 == *key1);
  fprintf (stderr, "ok\n");

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ColTypeKeyTest();
}
