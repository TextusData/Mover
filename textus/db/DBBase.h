/* DBBase.h -*- c++ -*-
 * Copyright (C) Ross Biro 2012
 *
 * Basic defines for use in the database code.
 */


#ifndef TEXTUS_DB_DB_BASE_H_
#define TEXTUS_DB_DB_BASE_H_

namespace textus { namespace db {

#define SRCI(x, y) do { ret = (x); switch (ret) { case SQL_SUCCESS: case SQL_SUCCESS_WITH_INFO: case SQL_NO_DATA_FOUND: break; default: \
	LOG(INFO) << "SQL Error (" << __FILE__ << ", " << __LINE__ << ") in statement: " #x  " = " << ret << y << "\n"; goto error_out;} \
} while (0)

#define SRC(x) SRCI((x), "")

#define SRCSTMT(x, hstmt) do { SQLCHAR state[6]; SQLINTEGER sqlerr; SQLCHAR message[200]; SQLSMALLINT messsize=sizeof(message); \
  SRCI((x), SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, (hstmt), state, &sqlerr, message, messsize, &messsize) << \
       " : " << state << " : " << sqlerr << " : " << message); } while (0)

#define SRCENV(x, henv) do { SQLCHAR state[6]; SQLINTEGER sqlerr; SQLCHAR message[200]; SQLSMALLINT messsize=sizeof(message); \
    SRCI((x), SQLError((henv), SQL_NULL_HDBC, SQL_NULL_HSTMT, state, &sqlerr, message, messsize, &messsize) << \
	 " : " << state << " : " << sqlerr << " : " << message); } while (0)

#define SRCDB(x, hdbc) do { SQLCHAR state[6]; SQLINTEGER sqlerr; SQLCHAR message[200]; SQLSMALLINT messsize=sizeof(message); \
    SRCI((x), SQLError(SQL_NULL_HENV, (hdbc), SQL_NULL_HSTMT, state, &sqlerr, message, messsize, &messsize) << \
	 " : " << state << " : " << sqlerr << " : "  << message); } while (0)
                    															      

#define SRNULL(x) do { if (!(x)) { goto error_out; } } while (0)

#define CCTOSQL(x) reinterpret_cast<SQLCHAR *>(const_cast<char *>(x))
#define STRTOSQL(x) CCTOSQL((x).c_str())

}} //namespace


#endif //TEXTUS_DB_DB_BASE_H_
