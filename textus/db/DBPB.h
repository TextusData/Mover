/* DBPB.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Template to make a protobuf into a database type.  Stored as a varbinary and virtually inaccessible outside the program.
 * Should only be used with types that really should be opaque like keys.  Otherwise use references to other tables
 * for compaound types.
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

#ifndef TEXTUS_DB_DBPB_H_
#define TEXTUS_DB_DBPB_H_

namespace textus { namespace db {

template <typename T, size_t maxlen = 16384>  class DBPB: virtual public ColBase {
private:
  SQLLEN len;
  SQLCHAR *pbdata;

public:
  explicit DBPB(): pbdata(NULL) {
    twopass = true;
  }

  explicit DBPB(const string s) {
    copyIn(s);
  }

  virtual ~DBPB() {}

  char htoc(const char *s) {
    char c = *s;
    return (c >= '0' && c <= '9') ? c-'0' : (c >= 'a' && c <= 'f') ? c-'a' + 10 : (c >= 'A' && c <= 'F') ? c- A + 10 : -1;
  }

  int htob(const char *s) {
    char n1 = htoc(s[0]);
    char n2 = htoc(s[1]);
    if (n1 < 0 || n2 < 0) {
      return -1;
    }
    return n1 * 16 + n2;
  }

  void copyIn(const string s) {
    len = min(s.length(), static_cast<size_t>(maxlen));
    
  }

  string sqlType() {
    string s = "varbinary";
    return s;
  }

  string toString() const {
    string s = copyOut();
    return s;
  }

  virtual int bindCol(DBStatement *stmt, int colnumber) {
    SQLRETURN sr = SQLBindCol(stmt->hstmt(), colnumber, SQL_C_BINARY, &pbdata, len, &len);
    return sr;
  }

  string value() const {
    return copyOut();
  }

  void setValue(const string str) {
    copyIn(str);
  }
  
};

}} // namespace

#endif // TEXTUS_DB_DBPB_H_
