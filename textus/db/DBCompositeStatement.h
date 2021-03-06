/* DBCompositeStatement.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Contains the classes that allow us to communicate with multiple db's at once.
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

#ifndef TEXTUS_DB_DBCOMPOSITESTATEMENT_H_
#define TEXTUS_DB_DBCOMPOSITESTATEMENT_H_

namespace textus { namespace db {

#define MAKEFUNC(func) int func() {			\
  int ret=0;						\
  foreach (i, statements) {				\
    HRC((*i)->func());					\
  }							\
error_out:						\
  return ret;						\
}

#define MAKEFUNC1(func, t1, v1) int func(t1 v1) {	\
  int ret=0;						\
  foreach (i, statements) {				\
    HRC((*i)->func(v1));				\
  }							\
error_out:						\
  return ret;						\
}

class DBCompositeStatement: public DBStatement {
private:
  ReferenceList<DBStatement *> statements;

public:
  DBCompositeStatement() {}
  virtual ~DBCompositeStatement() {}

  virtual int readonly() { return false; }
  
  void addStatement(DBStatement *s) {
    statements.push_back(s);
  }
  
  int execute() {
    LOG(ERROR) << "execute called on write statement.  Must use execute direct.\n";
    die();
    /* does not return. */
    return 0;
  }

  MAKEFUNC(unbindColumns);
  MAKEFUNC1(bindColNull, int, colnum);
  MAKEFUNC1(bindParams, ReferenceList<ColBase *> *, p);
  
};

#undef MAKEFUNC
#undef MAKEFUNC1

}} //namespace

#endif //TEXTUS_DB_DBCOMPOSITESTATEMENT_H_
