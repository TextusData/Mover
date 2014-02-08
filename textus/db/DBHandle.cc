/* DBHandle.cc -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 *
 * Implements a database handle,
 * a connection to a single database.
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

#include "textus/base/Base.h"
#include "textus/db/DBType.h"
#include "textus/db/DBHandle.h"
#include "textus/db/DBStatement.h"


namespace textus { namespace db {

#include "sql.h"

#define SRCME(x) SRCENV((x), handle)

DBHandle *DBInstance::getHandle() {
  Synchronized(this);
  return handle;
}

int DBInstance::weakDelete()
{
  setpool(NULL);
  handle->setinstance(NULL);
  handle = NULL;
  return Base::weakDelete();
}

int DBInstance::connect() {
  int ret;
  Synchronized(this);
  AUTODEREF(DBHandle *, h);
  h = getHandle();
  if (h == NULL) {
    h = new DBHandle();
    handle = h;
    handle->setinstance(this);
  } else {
    h->ref();
  }
  SRNULL(h);
  SRC(h->connect(*this));

 error_out:
  return ret;
}

void DBInstance::returnHandle(DBHandle *h)
{
    h->pool()->returnHandle(h);
}

#undef SRCME
#define SRCME(x) SRCDB((x), conn)

DBStatement *DBHandle::prepare(string sql) {
  DBStatement *st = new DBStatement();
  int ret;
  SRCME(SQLAllocHandle(SQL_HANDLE_STMT, conn, &st->handle));

  if (isReadHandle()) {
    HRC(st->readonly());
  }

  SRCSTMT(SQLSetStmtAttr(st->handle, SQL_ATTR_CURSOR_TYPE, reinterpret_cast<SQLPOINTER>(SQL_CURSOR_STATIC), SQL_IS_INTEGER), st->handle);

  SRCSTMT(SQLPrepare(st->handle, reinterpret_cast <SQLCHAR *> (const_cast <char *> (sql.c_str())), sql.length()), st->handle);
  return st;

 error_out:
  return NULL;
}

int DBHandle::connect(const DBInstance &dbi) 
{
  int ret;
  SQLCHAR *server;
  SQLCHAR *user;
  SQLCHAR *password;

  int server_len;
  int user_len;
  int password_len;

  if (dbi.server().find_first_of("=;") != string::npos) {
    HRC(driverConnect(dbi));
    goto done;
  }

  SRC(SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE, &env));
  SRCENV(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0), env);
  SRCENV(SQLAllocHandle(SQL_HANDLE_DBC, env, &conn), env);

  dbi.password().pin();

  // Break all this out to help with debugging.
  server_len = dbi.server().length();
  user_len = dbi.user().length();
  password_len = dbi.password().length();

  // Likely totally unnecessary.
  server = (server_len > 0) ? CCTOSQL(dbi.server().c_str()) : NULL;
  user = (user_len > 0) ? CCTOSQL(dbi.user().c_str()) : NULL;
  password = (password_len > 0) ? CCTOSQL(dbi.password().c_str()) : NULL;


  // XXXXX Fixme: Valgrind says this is a leak.
  SRCDB(SQLConnect(conn, server, server_len, user, user_len, password, password_len), conn);
  dbi.password().unpin();
  
  // separate this too label for debugging purposes.
 done:
  return 0;

 error_out:
  return (ret == SQL_ERROR);
}

int DBHandle::driverConnect(const DBInstance &dbi)
{
  SQLCHAR connectString[1024];
  SQLSMALLINT cs_size=sizeof(connectString)/sizeof(connectString[0]);
  int ret=0;

  SRC(SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE, &env));
  SRCENV(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0), env);
  SRCENV(SQLAllocHandle(SQL_HANDLE_DBC, env, &conn), env);

  // XXXXXX FIXME: valgrind says this uses an unititialized value.
  SRCME(SQLDriverConnect(conn, NULL /* hwnd */, CCTOSQL(dbi.server().c_str()), dbi.server().length(), connectString, cs_size,
			 &cs_size, SQL_DRIVER_NOPROMPT));

  // break this out for debugging purposes.
  return 0;

 error_out:
  return (ret == SQL_ERROR);
}

int DBHandle::close()
{
  int ret=0;
  Synchronized(this);
  if (conn != SQL_NULL_HANDLE) {
    SRCME(SQLDisconnect(conn));
    SRCME(SQLFreeHandle(SQL_HANDLE_DBC, conn));
    conn = SQL_NULL_HANDLE;
  }

  if (env != SQL_NULL_HANDLE) {
    SRCENV(SQLFreeHandle(SQL_HANDLE_ENV, env), env);
    env = SQL_NULL_HANDLE;
  }
 error_out:
  return ret;
}



}} //namespace
