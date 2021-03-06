/* DBUtils.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Misc Database utilties to do fun things like
 * create databases for unittests and run
 * scripts.
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

#ifndef TEXTUS_DB_DBUTILS_H_
#define TEXTUS_DB_DBUTILS_H_

#include "textus/base/Base.h"
#include "textus/base/AutoDeref.h"
#include "textus/db/DBHandle.h"
#include "textus/system/Environment.h"

namespace textus { namespace db {

using namespace std;

// Doesn't use the pooling, etc because we want to
// be able to do maintenance on a single db.

class DBUtils: virtual public Base {
private:
  AutoDeref<DBInstance> dbi;
  
public:
  DBUtils() {}
  virtual ~DBUtils() {}

  int init() {
    AUTODEREF(DBInstance *, i);
    i = new DBInstance();
    int ret = 0;
    HRNULL(i);
    dbi = i;

  error_out:
    return ret;
  }

  void setDSN(string dsn) {
    if (dbi == NULL) {
      LOG(INFO) << "DBUtils::setDSN called while dbi is NULL. Call Init First.\n";
      return;
    }
    dbi->setServer(dsn);
  }

  void setUser(string user) {
    if (dbi == NULL) {
      LOG(INFO) << "DBUtils::setUser called while dbi is NULL.  Call Init First.\n";
      return;
    }
    dbi->setUser(user);
  }

  int connect() {
    int ret = 0;
    HRC(dbi->connect());
  error_out:
    return ret;
  }

  int createDB(string name);
  int dropDB(string name);
  int executeScript(string scriptname, const textus::system::Environment *env = NULL);
  int executeSQL(string sql, const textus::system::Environment *env = NULL);
  int close();

};

}} //namespace


#endif // TEXTUS_DB_DBUTILS_H_
