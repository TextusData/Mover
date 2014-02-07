/* ColType.h -*- c++ -*-
 * Copyright (c) 2011-2013 Ross Biro
 * Contains the base classes that represents a column type as stored in a data base.
 */

#ifndef TEXTUS_DB_COLTYPE_H_
#define TEXTUS_DB_COLTYPE_H_

#include "textus/base/Base.h"
#include "textus/db/DBStatement.h"

#include <string>
#include <sstream>

namespace textus { namespace db {

// avoid poluting the namespace as much as possible.  #defines will still be everywhere though.
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>


using namespace textus::base;

class DBStatement;

class ColBase: virtual public Base {
private:
protected:
  SQLLEN len;

public:
  explicit ColBase(): len(SQL_NULL_DATA)  {}
  explicit ColBase(SQLLEN n): len(n) {}

  virtual string sqlType()=0;
  virtual int bindCol(DBStatement *, int colnumber) = 0;
  virtual int bindParam(DBStatement *, int colnumber) = 0;
  virtual int postFetch(int colnumber) { return 0; }
  virtual bool needPass2() { return false; }
  virtual int bindColPass2(DBStatement *, int colnumber, void *data, size_t max_size) { return 0; }
  virtual void pass2Data(void *d) {}

  bool isNull() const { return len == SQL_NULL_DATA; }
  size_t getLen() const { return len; }
};

template <SQLSMALLINT sqltype> class ColLobBase: public ColBase {
protected:
  AutoFree<char> data2;
  char *data;
  string fd;

public:
  ColLobBase() { len = 0; }
  virtual ~ColLobBase() {}
  
  virtual int postFetch(int colnumber) { fd = string(data, len); return 0; }

  virtual int bindCol(DBStatement *stmt, int colnumber) 
  {
    int ret;

    // Start by binding NULL.  On the second pass, we bind a pointer, once we know how
    // big we have to be.
    SRCSTMT(SQLBindCol(stmt->hstmt(), colnumber, SQL_C_CHAR, reinterpret_cast<SQLPOINTER>(&data), 
		       sizeof(data), &len), stmt->hstmt());
  error_out:
    return ret == SQL_ERROR;
  }

  virtual int bindColPass2(DBStatement *stmt, int colnumber, void *data, size_t maxlen) {
    int ret;
    SRCSTMT(SQLBindCol(stmt->hstmt(), colnumber, SQL_C_CHAR, data, maxlen, &len), stmt->hstmt());

  error_out:
    return ret == SQL_ERROR;
  }

  bool needPass2() {
    return true;
  }

  virtual void pass2Data(void *d) { data = static_cast<char *>(d); }

  virtual int bindParam(DBStatement *stmt, int paramnumber) 
  {
    int ret;
    SRCSTMT(SQLBindParameter(stmt->hstmt(), paramnumber, SQL_PARAM_INPUT, SQL_C_CHAR, sqltype, len, 0, data2, len, &len), stmt->hstmt());

  error_out:
    return ret == SQL_ERROR;
  }

  void copyIn(const string str) {
    len = str.length();
    int i=0;
    data2 = static_cast<char *>(malloc (len + 1));
    for (string::const_iterator it = str.begin(); it != str.end() && i < len; ++it, ++i) {
      data2[i] = *it;
    }
    data2[i] = 0;
  }
  
  string copyOut() const {
    return fd;
  }
};

class ColTypeLOB: public ColLobBase<SQL_VARCHAR> {
public:
  explicit ColTypeLOB() {}
  virtual ~ColTypeLOB() {}

  string value() const {
    return copyOut();
  }

  void setValue(string s) {
    copyIn(s);
  }

  string sqlType() {
    string s = "varchar";
    return s;
  }
};


template <int maxlen> class DBStringBase: public ColBase {
private:
  SQLCHAR data[maxlen + 1];

public:
  DBStringBase() {}
  DBStringBase(const string str) {
    copyIn(str);
  }
  virtual ~DBStringBase() {}

  void copyIn(const string str) {
    len = min(str.length(), static_cast<size_t>(maxlen));
    int i=0;
    for (string::const_iterator it = str.begin(); it != str.end() && i < len; ++it, ++i) {
      data[i] = *it;
    }
  }

  string copyOut() const {
    string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i) {
      s.push_back(data[i]);
    }
    return s;
  }

  string sqlType() {
    string s = "varchar";
    return s;
  }

  string toString() const { 
    string s = copyOut();
    return "\"" + s + "\""; 
  }
  
  virtual int bindCol(DBStatement *stmt, int colnumber) 
  {
    SQLRETURN sr = SQLBindCol(stmt->hstmt(), colnumber, SQL_C_CHAR, &data, maxlen+1, &len);
    return sr;
  }

  virtual int bindParam(DBStatement *stmt, int paramnumber) 
  {
    SQLRETURN sr = SQLBindParameter(stmt->hstmt(), paramnumber, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, maxlen, 0, &data, maxlen+1, &len);
    return sr;
  }
};

template <int maxLeng> class DBString: public DBStringBase<maxLeng> {
public:
  explicit DBString() {};
  virtual ~DBString() {};

  string value() const {
    return DBStringBase<maxLeng>::copyOut();
  }

  void setValue(const string str) {
    DBStringBase<maxLeng>::copyIn(str);
  }
};


template <int precision, int scale=0> class Number: public ColBase {
private:
  double v;

public:
  explicit Number(): ColBase(1) {}
  explicit Number(int n):v(n), ColBase(0) {}
  explicit Number(double d): v(d), ColBase(0) {}
  virtual ~Number() {}
  
  string sqlType() {
    stringstream ss;
    ss << "Number(" << precision << ", " << scale << ")";
    return ss.str();
  }
  double value() const { return v; }
  void setValue(double d) { v = d; }
  string toString() { string s = v; return s; }

  virtual int bindCol(DBStatement *stmt, int colnumber) {
    int ret;
    SRCSTMT(SQLBindCol(stmt->hstmt(), colnumber, SQL_C_DOUBLE, &v, sizeof(v), &len), stmt->hstmt());
  error_out:
    return ret==SQL_ERROR;
  }

  virtual int bindParam(DBStatement *stmt, int paramnumber) 
  {
    int ret;
    SRCSTMT(SQLBindParameter(stmt->hstmt(), paramnumber, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_NUMERIC, precision, scale, &v, 0, &len), 
	    stmt->hstmt());

  error_out:
    return ret==SQL_ERROR;
  }
    
};

template <int precision > class Number<precision, 0>: virtual public ColBase {
private:
  long v;

public:
  explicit Number(): ColBase(1) {}
  explicit Number(int n): v(n), ColBase(0) {}
  explicit Number(double d): v(d), ColBase(0) {}
  virtual ~Number() {}
  
  string sqlType() {
    stringstream ss;
    ss << "Number(" << precision << ", 0)";
    return ss.str();
  }

  long value() const { return v; }
  void setValue(long d) { v = d; }
  string toString() { string s = v; return s; }

  virtual int bindCol(DBStatement *stmt, int colnumber)
  {
    SQLRETURN sr = SQLBindCol(stmt->hstmt(), colnumber, SQL_C_LONG, &v, sizeof(v), &len);
    return sr;
  }

  virtual int bindParam(DBStatement *stmt, int paramnumber) 
  {
    SQLRETURN sr = SQLBindParameter(stmt->hstmt(), paramnumber, SQL_PARAM_INPUT, SQL_C_LONG, SQL_NUMERIC, precision, 0, &v, 0, &len);

    return sr;
  }

};

class DBInt: virtual public ColBase {
private:
  int v;

public:
  explicit DBInt():v(0) {}
  explicit DBInt(int n) { v = n;}
  explicit DBInt(double d) { v = d; }
  virtual ~DBInt() {}
  
  string sqlType() {
    string s = "Int";
    return s;
  }
  int value() const { return v; }
  void setValue(int d) { v = d; }
  string toString() const { 
    string s;
    stringstream out;
    out << v;
    s = out.str();
    return s; 
  }

  virtual int bindCol(DBStatement *stmt, int colnumber)
  {
    SQLRETURN sr = SQLBindCol(stmt->hstmt(), colnumber, SQL_C_LONG, &v, sizeof(v), &len);
    return sr;
  }

  virtual int bindParam(DBStatement *stmt, int paramnumber) 
  {
    SQLRETURN sr = SQLBindParameter(stmt->hstmt(), paramnumber, SQL_PARAM_INPUT, SQL_C_LONG, SQL_NUMERIC, sizeof(v), 0, &v, 0, &len);
    return sr;
  }

};


}} //namespace

#endif //TEXTUS_DB_COLTYPE_H_
