/* DBCompositHandle.h -*- c++ -*-
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

#ifndef TEXTUS_DB_DBCOMPOSITHANDLE_H_
#define TEXTUS_DB_DBCOMPOSITHANDLE_H_

namespace textus { namespace db {

class DBCompositHandle: public DBHandle {
private:
  ReferenceList<DBHandle *> handles;

public:
  DBCompositHandle() {};
  virtual ~DBCompositHandle() {}
  
  
  virtual bool isWriteHandle() {
    if (handles.empty()) {
      return false;
    } else {
      handles.first()->isWriteHandle();
    }
  }

  virtual DBStatement *prepare(string sql) {
    DBCompositeStatement *stmt = new DBCompositeStatement();
  }

};

}} //namespace


#endif // TEXTUS_DB_DBCOMPOSITHANDLE_H_
