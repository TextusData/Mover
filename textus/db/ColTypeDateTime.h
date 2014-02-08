/* ColTypeDateTime.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * A class to handle I/O of timestamps from
 * the database.
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

#ifndef TEXTUS_DB_COLTYPEDATETIME_H_
#define TEXTUS_DB_COLTYPEDATETIME_H_
#include "ColType.h"

namespace textus { namespace db {

class ColTypeDateTime: public ColBase {
private:
  SQL_TIMESTAMP_STRUCT sql_timestamp;

public:
  explicit ColTypeDateTime() {}
  virtual ~ColTypeDateTime() {}

  virtual int bindParam(DBStatement *stmt, int paramnumber) {
    int ret;
    SRCSTMT(SQLBindParameter(stmt->hstmt(), paramnumber, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
			     0, 0, &sql_timestamp, 0, &len), stmt->hstmt());
  error_out:
    return ret==SQL_ERROR;
  }

  virtual string sqlType() {
    return string("timestamp");
  }

  virtual int bindCol(DBStatement *stmt, int colnumber) {
    int ret;
    SRCSTMT(SQLBindCol(stmt->hstmt(), colnumber, SQL_C_TYPE_TIMESTAMP, &sql_timestamp, 0, &len), stmt->hstmt());
  error_out:
    return ret==SQL_ERROR;
  }

  // XXXXXXXX Fixme: what about timezones?
  Time *value() const {
    struct tm tm;
    tm.tm_year = sql_timestamp.year - 1900;
    tm.tm_mon = sql_timestamp.month-1; 
    tm.tm_mday = sql_timestamp.day;
    tm.tm_hour = sql_timestamp.hour;
    tm.tm_min = sql_timestamp.minute;
    tm.tm_sec = sql_timestamp.second;
    tm.tm_isdst = 0; // always use gmt.
    time_t t = mktime (&tm);
    // XXXXX Fixme use the fraction to round up.  But we need to know what the denominator is. */
    return new Time(t);
  }

  void setValue(Time *t) {
    struct tm tm;
    time_t tt = t->seconds();
    gmtime_r(&tt, &tm);
    memset(&sql_timestamp, 0, sizeof(sql_timestamp));
    sql_timestamp.year = tm.tm_year + 1900;
    sql_timestamp.month = tm.tm_mon + 1;
    sql_timestamp.day = tm.tm_mday;
    sql_timestamp.hour = tm.tm_hour;
    sql_timestamp.minute = tm.tm_min;
    sql_timestamp.second = tm.tm_sec;
  }
  
};


}} //namespace

#endif //TEXTUS_DB_COLTYPEDATETIME_H_
