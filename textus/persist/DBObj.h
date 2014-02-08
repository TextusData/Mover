/* DBObj.h -*- C++ -*- 
 * Copyright (C) 2010 Ross Biro
 * All persistant DB objects should inherit from this class.
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

#ifndef TEXTUS_PERSIST_DB_DBOBJ_H_
#define TEXTUS_PERSIST_DB_DBOBJ_H_

namespace textus { namespace persist { namespace db {

/*! \class DBObj DBObj represents the base class from which all
 * persistant objects that may be stored in a database should be
 * derived.
 *
 * Key things: It needs to know a default table to store the class in,
 * and all the other objects that need to be grouped into a single
 * transaction.  It also needs to know about any dependencies that
 * don't necessarily need to be updated in a single transaction.
 */

class DBObject: virtual public Base {
private:
  AutoDeref<class DBHandle> db; // database to store this thing in.

public:
  explicit DBObject() {}
  virtual ~DBObject() {}
  
};

}}} //namespace

#endif // TEXTUS_PERSIST_DB_DBOBJ_H_
