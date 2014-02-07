/* DBStatement.h -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 *
 * Implementation of  DBStatement class
 */

#include "textus/db/ColType.h"
#include "textus/db/DBStatement.h"

#define SRCME(x) SRCSTMT((x), handle)

namespace textus { namespace db {

int DBStatement::readonly() 
{
  int ret;
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_INTEGER));
  return 0;

 error_out:
  return -1;
}

int DBStatement::bindParams(ReferenceList<ColBase *> *p) {
  int ret=0;
  int ind = 0;
  for (ReferenceList<ColBase *>::iterator i = p->begin(); i != p->end(); ++i) {
    SRCME((*i)->bindParam(this, ++ind));
  }
 error_out:
  return ret;
}

int DBStatement::bindParams(DBType **t, size_t count) {
  int ret=0;

  SQLUSMALLINT *row_status;

  row_status = reinterpret_cast<SQLUSMALLINT *>(alloca(sizeof(*row_status) * count));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_ARRAY_SIZE, reinterpret_cast<SQLPOINTER>(count), SQL_IS_INTEGER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_BIND_OFFSET_PTR, reinterpret_cast<SQLPOINTER>(0), SQL_IS_POINTER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN, SQL_IS_INTEGER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROWS_FETCHED_PTR, reinterpret_cast<SQLPOINTER>(&count), SQL_IS_POINTER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_STATUS_PTR, reinterpret_cast<SQLPOINTER>(row_status), SQL_IS_POINTER));

 error_out:
  return ret;
}

int DBStatement::fetchDataByColumn(size_t &count) {
  int ret;
  SQLUSMALLINT *row_status;

  row_status = reinterpret_cast<SQLUSMALLINT *>(alloca(sizeof(*row_status) * count));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_ARRAY_SIZE, reinterpret_cast<SQLPOINTER>(count), SQL_IS_INTEGER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_BIND_OFFSET_PTR, reinterpret_cast<SQLPOINTER>(0), SQL_IS_POINTER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN, SQL_IS_INTEGER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROWS_FETCHED_PTR, reinterpret_cast<SQLPOINTER>(&count), SQL_IS_POINTER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_STATUS_PTR, reinterpret_cast<SQLPOINTER>(row_status), SQL_IS_POINTER));

  SRCME(SQLFetchScroll(handle, SQL_FETCH_ABSOLUTE, 1)); // There is no 0 row. // XXXXXXX Fixme, we need to make relative work.

  return 0;

 error_out:
  return -1;

}

int DBStatement::fetchData(DBType *result, size_t &count) {
  int ret;
  SQLUSMALLINT *row_status;

  row_status = reinterpret_cast<SQLUSMALLINT *>(alloca(sizeof(*row_status) * count));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_ARRAY_SIZE, reinterpret_cast<SQLPOINTER>(count), SQL_IS_INTEGER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_BIND_OFFSET_PTR, reinterpret_cast<SQLPOINTER>(0), SQL_IS_POINTER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_BIND_TYPE, reinterpret_cast<SQLPOINTER>(result->getSize()), SQL_IS_INTEGER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROWS_FETCHED_PTR, reinterpret_cast<SQLPOINTER>(&count), SQL_IS_POINTER));
  SRCME(SQLSetStmtAttr(handle, SQL_ATTR_ROW_STATUS_PTR, reinterpret_cast<SQLPOINTER>(row_status), SQL_IS_POINTER));

  SRCME(SQLFetch(handle));

  return 0;

 error_out:
  return -1;
    
}

int DBStatement::execute() {
  int ret;
  //XXXXX FIXME:  Valgrind says ODBC call uses unitialized variable.
  SRCME(SQLExecute(handle));
 error_out:
  return (ret == SQL_ERROR);
}

int DBStatement::bindColNull(int colnum)
{
  int ret;
  SRCME(SQLBindCol(handle, colnum, SQL_C_DEFAULT, NULL, 0, NULL));

 error_out:
  return ret == SQL_ERROR;
}

int DBStatement::unbindColumns() {
  int ret;
  SRCME(SQLFreeStmt(handle, SQL_UNBIND));
 error_out:
  return ret == SQL_ERROR;
}

}}//namespace
