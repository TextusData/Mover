 /* DBAutoDeref.h -*- c++ -*-
  * Copyright (c) 2012-2013 Ross Biro
  *
  * This one is fun.  It does a lazy bind to a db type.  So that 
  * it holds a guid, presumably from a table and fetches it on the first
  * use.  
  * 
  * Note, no effort is made to track or sync any changes to what we point to.
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

 #ifndef TEXTUS_DB_DBAUTODEREF_H_
 #define TEXTUS_DB_DBAUTODEREF_H_

 #include "textus/base/Base.h"
 #include "textus/base/AutoDeref.h"
 #include "textus/db/DBType.h"
 #include "textus/db/ColTypeUUID.h"

 namespace textus { namespace db {

 // T must descend from DBType.h, or at least use the same API.
 template <class T> class DBAutoDeref: public ColTypeUUID {
 private:
   AutoDeref<T> d;

 public:
   DBAutoDeref() {}
   virtual ~DBAutoDeref() {}

   T *refValue() {
     return refValue(DBHandle::defaultHandle());
   }

   virtual T *refValue(DBHandle *handle) {
     int ret = -1;
     string sql;
     ReferenceList<ColBase *> r;
     AUTODEREF(T *, tptr);
     if (d != NULL) {
       goto error_out;
     }
     tptr = new T();
     d = tptr;
     if (d == NULL) {
       goto error_out;
     }
     sql = d->sql(dbQuery);
     r.push_back(this);
     ret = DBType::query(sql, 1, d, handle, &r);

  error_out:
    if (ret != 1) {
      LOG(INFO) << "DBAutoDeref::data() faliure.  Returning NULL.\n";
      return NULL;
    }
    return d;
  }
};


}} //namespace

#endif // TEXTUS_DB_DBAUTODEREF_H_
