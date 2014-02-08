/* DBType.cc -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 * Contains the base classes that represents a type as stored in a data base.
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

#include "textus/db/DBType.h"

#include <string>
#include <vector>

namespace textus { namespace db {

using namespace std;

static string join(vector<string> l, string sep=" ") {
  bool first=true;
  string res;
  for (vector<string>::iterator i = l.begin(); i != l.end(); ++i) {
    if (!first) {
      res = res + sep;
    } else {
      first = false;
    }
    res = res + *i;
  }
  return res;
}

static string joinRepl(vector<string> l, string val, string sep=" ") {
  bool first=true;
  string res;
  for (vector<string>::iterator i = l.begin(); i != l.end(); ++i) {
    if (!first) {
      res = res + sep;
    } else {
      first = false;
    }
    res = res + val;
  }
  return res;
}

string DBType::sql(DBAction act, string where_clause) {
  string res;
  vector<string> cols;
  const ColumnMap cm = columns();
  for (ColumnMap::const_iterator i = cm.begin(); i != cm.end(); ++i) {
    string colName = i->getName();
    if (!enableColumn(colName, act)) {
      continue;
    }
    cols.push_back(colName);
  }

  if (cols.empty()) {
    LOG(ERROR) << "No columns to insert in DBType for table " << table() <<  endl;
    die();
  }
  
  switch (act) {
  default:
  case invalidAction:
    LOG(ERROR) << "unknown action: " << act << " in DBType::sql" << std::endl;
    die();
    return "";

  case dbQuery:
    res = "select " + join(cols, ", ") + " from " + table();
    if (where_clause.length()) {
      res += " where " + where_clause;
    } else if (primaryKey().length()) {
      res = res + " where " + primaryKey() + " = ?";
    }
    res = res + ";";
    return res;

  case dbInsert:
    if (where_clause.length()) {
      LOG(WARNING) << "where clause ignored for insert: " << where_clause << endl;
    }
    res = "insert into " + table() + " ( " + join(cols, ", ") + " ) values ( " + joinRepl(cols, "?", ", ") + ");";
    return res;

  case dbUpdate:
    res = "update " + table() + " set " + join (cols, " = ?, ") + " = ?";
    if (where_clause.length()) {
      res = res + " where " + where_clause;
    } else if (primaryKey().length()) {
      res = res + " where " + primaryKey() + " = ?";
    } else {
      LOG(ERROR) << "where clause or primary key required for update.\n";
      die();
    }
    res = res + ";";
    return res;
  }

  // Can't get here without a compiler bug.
  assert("compiler_bug" == NULL);
  return "";
}

string DBType::primaryKey() {
  ClassDescription *cd = classDesc();
  return cd->primaryKey();
};

void DBType::allocMem(size_t s) {
  void *vptr = malloc(s + sizeof(MemBase));
  MemBase *m;
  m = new (vptr) MemBase();
  m->next = reinterpret_cast<unsigned char *>(m + 1);
  m->size = s;
  current_base = m;
#ifdef DEBUG_DB_ALLOC
  magic = DB_BASE_MAGIC;
#endif
}

void DBType::dbTypeDelete(void *vptr) {
  if (delete_base != NULL) {
#ifdef DEBUG_DB_ALLOC
    assert((unsigned long)vptr > (unsigned long)delete_base && (unsigned long)vptr < (unsigned long)delete_base + delete_base->size);
    assert(delete_base->magic == DB_BASE_MAGIC);
#endif
    delete_base->lock.lock();
    MemBase * m = delete_base;
    if (--m->count == 0) {
      m->lock.unlock();
      /*delete m;  uses placement new to allocate it, so
		 * delete doesn't free any memory.
		 */
      free(m);
    }
    delete_base->lock.unlock();
    return;
  }
  free(vptr);
  return;
}

int DBType::query(string sql, size_t maxrows, DBType *result, DBPool *pool, ColBase *param) {
  ReferenceList<ColBase *> r;
  r.push_front(param);
  return query(sql, maxrows, result, pool, &r);
}


int DBType::query(string sql, size_t maxrows, DBType *result, DBPool *pool, ReferenceList<ColBase *> *p) {
  // Could block a long time here.
  return pool->query(sql, maxrows, result, p);
}

int DBType::query(string sql, size_t maxrows, DBType *result, DBHandle *db, ReferenceList<ColBase *> *p) {
  int ret;
  size_t count = maxrows;
  size_t pass2rowcount;
  void **data = NULL;

  AUTODEREF(DBStatement *, st);
  st = db->prepare(sql);
  HRNULL(st);
  
  /* bind the parameters if any. */
  if (p != NULL && !p->empty()) {
    HRC(st->bindParams(p)); 
  }

  HRC(st->execute());

  if (result == NULL || maxrows == 0) {
    count = 0;
    ret = 0;
    goto error_out;
  }

  // binds all the columns.
  HRC(result->bindColumns(st, dbQuery));
  HRC(st->fetchData(result, count));

  HRC(st->unbindColumns());
  pass2rowcount = result->bindColumnsPass2(st, dbQuery, count, data);
  if (pass2rowcount > 0) {
    HRC(st->fetchDataByColumn(count));
  }

  HRC(result->postQuery(count, maxrows)); // do this now so we can reduce the total count.

 error_out:
  if (data) {
    for (unsigned i = 0; i < pass2rowcount; ++i) {
      if (data[i]) {
	free(data[i]);
      }
    }
    free(data);
  }

  if (ret != 0) {
    return -1;
  }
  return count;
}

int DBType::bindParams(DBStatement *st, DBAction act) {
  return bindParams(st, act, NULL);
}

int DBType::bindParams(DBStatement *st, DBAction act, int *ordering) {
  int colnum = 0;
  int ret=0;
  for (ColumnMap::const_iterator i = columns().begin(); i != columns().end(); ++i) {
    if (!enableColumn(i->getName(), act)) {
      continue;
    }
    ColBase *cb = i->getBase(this);
    if (ordering) {
      HRC(cb->bindParam(st, ordering[++colnum]));
    } else {
      HRC(cb->bindParam(st, ++colnum));
    }
  }
 error_out:
  return ret;
}

int DBType::bindColumns(DBStatement *st, DBAction act) {
  int colnum = 0;
  int ret=0;
  for (ColumnMap::const_iterator i = columns().begin(); i != columns().end(); ++i) {
    if (!enableColumn(i->getName(), act)) {
      continue;
    }
    ColBase *cb = i->getBase(this);
    HRC(cb->bindCol(st, ++colnum));
  }
 error_out:
  return ret;
}

int DBType::bindColumnsPass2(DBStatement *st, DBAction act, size_t count, void **&data) {
  int colnum = 0;
  int pass2colnum = 0;
  int ret=0;
  data = NULL;

  // First see if we need to get any columns, and then see which ones.
  for (ColumnMap::const_iterator i = columns().begin(); i != columns().end(); ++i) {
    if (!enableColumn(i->getName(), act)) {
      continue;
    }
    ColBase *cb = i->getBase(this);
    if (cb->needPass2()) {
      ++pass2colnum;
    }
  }

  if (pass2colnum == 0) {
    ret = 0;
    goto error_out;
  }

  data = static_cast<void **>(malloc (sizeof(*data) * pass2colnum));
  HRNULL(data);
  memset(data, 0, sizeof(*data) * pass2colnum);
  pass2colnum = 0;

  for (unsigned j = 0; j < count; ++j) {
    for (ColumnMap::iterator i = columns().begin(); i != columns().end(); ++i) {
      if (!enableColumn(i->getName(), act)) {
	continue;
      }
      ColBase *cb = i->getBase(this);
      if (cb->needPass2()) {
	size_t len = cb->getLen();
	if (len == (size_t)SQL_NULL_DATA) {
	  continue;
	}
	if (cb->getLen() > i->maxSize()) {
	  i->setMaxSize(cb->getLen());
	}
      }
    }
  }

  // Allocate memory for each column, and bind it.
  for (ColumnMap::const_iterator i = columns().begin(); i != columns().end(); ++i) {
    if (!enableColumn(i->getName(), act)) {
      continue;
    }
    ColBase *cb = i->getBase(this);
    if (cb->needPass2()) {
      data[pass2colnum] = malloc(i->maxSize() * count);
      HRNULL(data[pass2colnum]);
      cb->pass2Data(data[pass2colnum]);
      HRC(cb->bindColPass2(st, ++colnum, data[pass2colnum], i->maxSize()));
      ++pass2colnum;
    } else {
      HRC(st->bindColNull(++colnum));
    }
  }

  return pass2colnum;

 error_out:
  return ret;
}

void DBType::postFetch(DBAction act) {
  int ret = 0;
  int colnum = 0;
  for (ColumnMap::const_iterator i = columns().begin(); i != columns().end(); ++i) {
    if (!enableColumn(i->getName(), act)) {
      continue;
    }
    ColBase *cb = i->getBase(this);
    HRC(cb->postFetch(++colnum));
  }
 error_out:
  return;
}

ThreadLocal<DBType::MemBase *> DBType::current_base;
ThreadLocal<DBType::MemBase *> DBType::delete_base;


}} //namespace
