/* DBCertificateAuthority.h -*- C++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 * Has the DB version of a certificate authority.
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

#ifndef TEXTUS_KEYS_DB_DB_CERTIFICATE_AUTHORITY_H_
#define TEXTUS_KEYS_DB_DB_CERTIFICATE_AUTHORITY_H_

#include "textus/db/ColType.h"
#include "textus/db/DBType.h"
#include "textus/db/ColTypeUUID.h"

namespace textus { namespace keys { namespace db {

using namespace textus::db;

class DBCertificateAuthority: public DBType {
  BEGIN_DB_DECL(DBCertificateAuthority);
  DECL_DB_ENTRY(DBCertificateAuthority, UUID, ColTypeUUID, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(DBCertificateAuthority, string, ColTypeLOB, name, 0);
  DECL_DB_ENTRY(DBCertificateAuthority, string, ColTypeLOB, id, 0);
  DECL_DB_ENTRY(DBCertificateAuthority, string, ColTypeLOB, certificate, 0);
  END_DB_DECL(DBCertificateAuthority);

public:
  DBCertificateAuthority() {}
  virtual ~DBCertificateAuthority() {}

  virtual string table() { return string("cas"); }
}; 

}}} //namespace


#endif //TEXTUS_KEYS_DB_DB_CERTIFICATE_AUTHORITY_H_
