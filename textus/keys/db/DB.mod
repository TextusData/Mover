cc_sources = [ textus/keys/db/DBCertificateAuthority.cc ],
#proto_sources = [ ],
target=keysdb.a,
deps=[ textus/base/Base, textus/logging/Logging, textus/db/DB ],
#unittests=[]
