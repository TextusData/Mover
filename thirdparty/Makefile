# /Makefile 
# Copyright (c) 2009-2014 Ross Biro
# 
# ThirdParty Makefile to build all the third party tools
#
all: protobuff openssl postgresql unixodbc KeccakCodePackage gpg libgpg-error
.PHONY: protobuf openssl doxygen postgresql unixodbc iodbc all KeccakCodePackage gpg libgpg-error

gpg: libgpg-error

protobuff:
	test -e $(BINROOT)/thirdparty/protobuf/bin/protoc || ( cd protobuf-2.4.1 && make -f Makefile.textus )

openssl:
	 test -e $(BINROOT)/thirdparty/openssl/lib/libssl.a || ( cd openssl-1.0.1c && make -f Makefile.textus )

doxygen:
	 test -e $(BINROOT)/thirdparty/doxygen/bin/doxygen || ( cd doxygen-1.8.4 && make -f Makefile.textus )

postgresql:
	 test -e $(BINROOT)/thirdparty/pgsql/lib/libpq.a || ( cd postgresql-9.0.1 && make -f Makefile.textus )

iodbc:
	 test -e $(BINROOT)/thirdparty/iodbc/lib/libiodbc.a || ( cd libiodbc-3.52.7 && make -f Makefile.textus )

unixodbc:
	 test -e $(BINROOT)/thirdparty/unixodbc/lib/libodbc.la || ( cd unixODBC-2.3.1 && make -f Makefile.textus )

KeccakCodePackage:
	test -e $(BINROOT)/thirdparty/KeccakCodePackage/lib/libkeccak.a || (cd KeccakCodePackage && make -f Makefile.textus)

dante:
	test -e $(BINROOT)/thirdparty/dante/lib/libdante.a || (cd dante-1.3.2 && make -f Makefile.textus)

gpg:
	test -e $(BINROOT)/thirdparty/gpg/bin/gpg2 || (cd gnupg-2.0.22 && make -f Makefile.textus)

libgpg-error:
	test -e $(BINROOT)/thirdparty/libgpg-error/lib/libgpg-error.so.0 || (cd libgpg-error-1.12 && make -f Makefile.textus)
