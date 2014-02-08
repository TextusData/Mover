/* DBType.h -*- c++ -*-
 * Copyright (c) 2010-2011, 2013 Ross Biro
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

#ifndef TEXTUS_DB_DBTYPE_H_
#define TEXTUS_DB_DBTYPE_H_

#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/base/locks/Mutex.h"
#include "textus/base/ThreadLocal.h"
#include "textus/base/UUID.h"
#include "textus/db/DBHandle.h"
#include <string>
#include <list>

namespace textus { namespace db {
using namespace std;
using namespace textus::base;

class DBType;
class DBPool;
class DBStatement;
class ColTypeUUID;
class DBHandle;

enum DBAction {
  invalidAction = 0,
  dbQuery,
  dbInsert,
  dbUpdate,
};

enum DBTypeFlags {
  DT_NO_FLAGS		=	0x00,
  DT_PRIMARY_KEY	=	0x01,
};

class DBType;
class ColBase;

class ColumnInfo {
private:
  const char *name;
  ColBase *(*f)(DBType *);
  int flags;
  int max_size; // used by variable size fields (i.e. varchar) to keep track of how big we've seen for this column.

public:
  explicit ColumnInfo(): max_size(0) {}
  explicit ColumnInfo(const char *n, ColBase *(*fp)(DBType *), int fl): name(n), f(fp), flags(fl), max_size(0) {}
  virtual ~ColumnInfo() {}

  const char *getName() const {
    return name;
  }

  ColBase* getBase(DBType *t) const {
    return f(t);
  }

  size_t maxSize() const { return max_size; }

  void setMaxSize(size_t ms) { max_size = ms; }
};

class ClassDescription: virtual public Base {
public:
  typedef DBType * (*Allocator)(size_t count);

private:
  list<ColumnInfo> ci;
  const char *pk;
  Allocator alloc;

public:
  explicit ClassDescription() {}
  explicit ClassDescription(Allocator a): alloc(a) {}
  virtual ~ClassDescription() {}

  void addColumn(const char *name, ColBase *(*f)(DBType *), int flags) { 
    struct ColumnInfo c(name, f, flags); 
    ci.push_back(c);
    if ((flags & DT_PRIMARY_KEY) != 0) {
      if (pk != NULL) {
	LOG(ERROR) << " Multiple Primary Keys for type.\n";
	die();
      }
      pk = name;
    }
  }
  
  string primaryKey() const {
    return string(pk);
  }

  list<ColumnInfo> &infoList() { return ci; }

  void setAllocator(Allocator a) {
    alloc = a;
  }

};

class DBType: virtual public Base {
private:
  struct MemBase {
#ifdef DEBUG_DB_ALLOC
    uint32_t magic;
#endif
    class locks::Mutex lock;
    size_t object_size;
    size_t size; // how much memory we have left
    size_t count; // how many are currently allocated.
    unsigned char *next;
  } *membase;

  static ThreadLocal<MemBase *> current_base;
  static ThreadLocal<MemBase *> delete_base;

protected:
  typedef list<ColumnInfo> ColumnMap;

  static void allocMem(size_t m); // save's it so operator new uses it.
  
  virtual ClassDescription *classDesc() { assert (0); }
  virtual ColumnMap &columns() {
    ClassDescription *cd = classDesc();
    return cd->infoList();
  }
  virtual int postQuery(size_t count, size_t maxrows) = 0;
  virtual string table()=0; // Default table for the type.
  virtual string primaryKey(); // return the name of the primary key.
  
  virtual bool enableColumn(string column, DBAction act) { return true; } // override if you don't always want to read/write all columns.

  explicit DBType() { membase = current_base; } // we shouldn't create these directly.  We create a derived type.

public:
  static int query(string sql, size_t maxrows, DBType *result, DBPool *pool, ColBase *param);
  static int query(string sql, size_t maxrows, DBType *result, DBPool *pool, ReferenceList<ColBase *> *params=NULL);
  static int query(string sql, size_t maxrows, DBType *result, DBHandle *h, ReferenceList<ColBase *> *params=NULL);
  

  virtual string sql(DBAction act, string where_clause=""); // Default SQL statement for the type.
  virtual ~DBType() {  delete_base = membase; }

  virtual size_t getSize() = 0;

  // Can't use placement new because then the compiler
  // won't automatically call delete.  We need someone
  // to call delete, so we can make sure that
  // we free the memory as it is no longer used.
  static void *dbTypeNew(size_t s) {
    if (current_base) {
#ifdef DEBUG_DB_ALLOC
      assert((unsigned long)vptr > (unsigned long)current_base && 
	     (unsigned long)vptr < (unsigned long)current_base + current_base->size);
      assert(current_base->magic == DB_BASE_MAGIC);
      assert(current_base->object_size == s);
#endif
      if (current_base->size) {
	void *vptr = static_cast<void *>(current_base->next);
	current_base->next += s;
	current_base->size -= s;
	// have to lock becuause someone else could be decrementing count.
	// really neead atomic types.
	current_base->lock.lock();
	current_base->count++;
	current_base->lock.unlock();
	return vptr;
      }
      current_base = NULL;
    }
    return malloc(s);
  }

  virtual int bindColumns(DBStatement *, DBAction);
  virtual int bindParams(DBStatement *, DBAction);
  virtual int bindParams(DBStatement *st, DBAction act, int *ordering);
  virtual int bindColumnsPass2(DBStatement *st, DBAction act, size_t count, void **&data);
  virtual void postFetch(DBAction);

  static void dbTypeDelete(void *vptr); 
};


#define BEGIN_DB_DECL(cls)						\
  private:								\
  static textus::db::ClassDescription classDescription;			\
  static DBType *alloc(size_t);						\
  static size_t max_size;						\
protected:								\
 int postQuery(size_t count, size_t maxrows);				\
public:									\
 virtual ClassDescription *classDesc() { return &classDescription; }	\
 virtual size_t getSize() { return sizeof(cls); }			\
 static cls *allocResultSet(size_t s);					\
 static cls *queryByPrimaryKey(const UUID uuid);			\
 void operator delete(void *vptr) { DBType::dbTypeDelete(vptr); }	\
 void *operator new(size_t s) { return DBType::dbTypeNew(s); }	\

    
#define DECL_DB_ENTRY(cls, ctype, dbtype, colname, flags)		\
  private:								\
  class colname##RegisterColumn {					\
  private:								\
     static int registerMe(ClassDescription *cld,			\
			   ColBase *(*f)(DBType *),			\
			   const char *n,				\
			   int fl) {					\
       cld->addColumn(n, f, fl);					\
       return 0;							\
     }									\
  public:								\
  colname##RegisterColumn(class ClassDescription *cd, const char *nme,	\
			  class ColBase *(*func)(class DBType *),	\
			  int fl					\
			  ) {						\
    static int doOnce __attribute__((unused))=registerMe(cd, func, nme, fl);\
  }									\
  };									\
  static const char *colname##Name() { return #colname; }		\
  static ColBase *colname##td_func(DBType *dbt) {			\
    cls *c = dynamic_cast<cls *>(dbt);					\
    assert (c != NULL);							\
    return &c->colname##Data;						\
  }									\
  dbtype colname##Data;							\
  friend class colname##Registerclass;					\
  /* can't be static or we would have to someone get the instantiation	\
     into the c++ file. */						\
  class colname##RegisterClass {					\
    colname##RegisterColumn internal;					\
  public:								\
  colname##RegisterClass(): internal(&cls::classDescription,		\
				 #colname,				\
				 colname##td_func,			\
				 flags) {}				\
  } colname##internal;							\
  									\
public:									\
 ctype colname() const { return colname##Data.value(); }		\
 void set_##colname(ctype v) { colname##Data.setValue(v); }		\
 dbtype *get_column_##colname() {return &colname##Data; }		\
 int get_length_##colname() { return colname##Data.getLen(); }		\
private:	


#define END_DB_DECL(class) private:

#define VA_ARGS_EMPTY_IMPL(x1, x2, x3, x4, x5, x6, x7, n, ...) n
#define VA_ARGS_EMPTY_SHIFT(...) VA_ARGS_EMPTY_IMPL(__VA_ARGS__, 1, 1, 1, 1, 1, 0, 0)
#define VA_ARGS_EMPTY(...) VA_ARGS_EMPTY_SHIFT(x, ##__VA_ARGS__)
#define VA_ARGS_FIRST(x, ...) x
#define VA_ARGS_BUT_FIRST(x, ...) __VA_ARGS__

#define MACRO_LOOP1(base, ...) base##1(VA_ARGS_FIRST(__VA_ARGS__)) MACRO_LOOP_B(base, VA_ARGS_BUT_FIRST(__VA_ARGS__))
#define MACRO_LOOP0(base, ...) base##0(VA_ARGS_FIRST(__VA_ARGS__))
#define MACRO_LOOP_IMPL2(n, base, ...) MACRO_LOOP##n(base, __VA_ARGS__)
#define MACRO_LOOP_IMPL1(n, base, ...) MACRO_LOOP_IMPL2(n, base, __VA_ARGS__)
#define MACRO_LOOP_B_IMPL2(n, base, ...) MACRO_LOOP_B##n(base, __VA_ARGS__)
#define MACRO_LOOP_B_IMPL(n, base, ...) MACRO_LOOP_B_IMPL2(n, base, __VA_ARGS__)
#define MACRO_LOOP_B(base, ...) MACRO_LOOP_B_IMPL(VA_ARGS_EMPTY(__VA_ARGS__), base, __VA_ARGS__)
#define MACRO_LOOP_B0(base, ...) base##0(VA_ARGS_FIRST(__VA_ARGS__))
#define MACRO_LOOP(base, ...) MACRO_LOOP_IMPL1( VA_ARGS_EMPTY(__VA_ARGS__), base, __VA_ARGS__)
#define MACRO_LOOP_B1(base, ...) , base##1(VA_ARGS_FIRST(__VA_ARGS__)) MACRO_LOOP_B(base, VA_ARGS_BUT_FIRST(__VA_ARGS__))


#define STATIC_DB_DECL(cls) PREFIX_STATIC_DB_DECL(cls,)
#define TEMPLATE_DB_DECL(cls, ...) PREFIX_STATIC_DB_DECL(cls, template < MACRO_LOOP(TYPENAME_PREFIX, __VA_ARGS__) >, < __VA_ARGS__ >)
#define TYPENAME_PREFIX1(x) typename x
#define TYPENAME_PREFIX0(x) , typename x
#define PREFIX_STATIC_DB_DECL(cls, prefix, ...)				\
  prefix textus::db::ClassDescription cls __VA_ARGS__::classDescription(cls __VA_ARGS__::alloc); \
  prefix size_t cls __VA_ARGS__::max_size;					\
  prefix cls __VA_ARGS__ *cls __VA_ARGS__::allocResultSet(size_t count) {			\
    return dynamic_cast<cls *>(alloc(count));				\
  }									\
  prefix int cls __VA_ARGS__::postQuery(size_t count, size_t maxrows) {		\
    for (size_t i = 0; i < maxrows; ++i) {				\
      if (i < count) {							\
        this[i].postFetch(dbQuery);					\
      } else {								\
        this[i].deref();						\
      }									\
    }									\
    return 0;								\
  }									\
  prefix DBType *cls __VA_ARGS__::alloc(size_t count) {				\
    allocMem(count * sizeof(cls));					\
    cls *c=NULL;							\
    cls *n;								\
    for (unsigned i = 0; i < count; ++i) {				\
      if (c == NULL) {							\
	c = new(cls);							\
      } else {								\
	n = new (cls);							\
	assert (n == c+i);						\
      }									\
    }									\
    return c;								\
  }									\
  prefix cls __VA_ARGS__ *cls __VA_ARGS__::queryByPrimaryKey(const UUID uuid) {			\
    int ret = 0;							\
    string sql;								\
    ColTypeUUID dbuuid;							\
    ReferenceList<ColBase *> r;						\
    cls *d = new cls();							\
    HRNULL(d);								\
    sql = d->sql(dbQuery);						\
    dbuuid.setValue(uuid);						\
    r.push_back(&dbuuid);						\
    HRC(DBPool::getPool()->query(sql, 1, d, &r));			\
    if (ret > 1) {							\
      LOG(ERROR) << "query for " #cls " by primary key returned "	\
                 << ret << " results\n";				\
      die();								\
    }									\
error_out:								\
    if (d != NULL && ret < 0) {						\
      delete d;								\
      d = NULL;								\
    }									\
    r.clear();								\
    return d;								\
  }									\
    

  	
}} //namespace

#endif //TEXTUS_DB_DBTYPE_H_
