/* DBCertificateAuthority.h -*- C++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 * Has the DB version of a certificate authority.
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
