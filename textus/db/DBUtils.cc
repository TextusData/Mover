/* DBUtils.cc -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
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

#include "textus/db/DBUtils.h"
#include "textus/file/LineReader.h"

namespace textus { namespace db {

using namespace std;

int DBUtils::executeSQL(string sql, const textus::system::Environment *env)
{
  DBHandle *h = dbi->getHandle();
  AUTODEREF(DBStatement *,statement);

  if (env != NULL) {
    sql = env->expandStrings(sql);
  }

  int ret = 0;
  HRNULL(h);
  statement = h->prepare (sql);
  HRNULL(statement);

  HRC(statement->execute());

 error_out:
  return ret;

}

int DBUtils::createDB(string name) 
{
  int ret=0;
  HRC(executeSQL(string("create database ") + name));
  
 error_out:
  return ret;
}

int DBUtils::dropDB(string name)
{
  int ret=0;
  HRC(executeSQL(string("drop database ") + name));
  
 error_out:
  return ret;
}

int DBUtils::close() 
{
  dbi=NULL;
  return 0;
}

int DBUtils::executeScript(string name, const textus::system::Environment *env) 
{
  AUTODEREF(textus::file::LineReader *, lr);
  AUTODEREF(textus::file::FileHandle *, fh);
  int ret=0;

  fh = textus::file::FileHandle::openFile(name, O_RDONLY);
  HRNULL(fh);
  lr = new textus::file::LineReader(fh);
  HRNULL(lr);

  while (!lr->eof()) {
    string sql = lr->waitForLine();
    size_t s = sql.find ("--");
    if (s != string::npos) {
      sql = sql.substr(0, s);
    }
    s = sql.find_first_not_of(" \t\r\n");
    if (s == string::npos) {
      continue;
    }
    HRC(executeSQL(sql, env));
  }

  fh->close();
  
 error_out:
  return ret;
}

}} //namespace
