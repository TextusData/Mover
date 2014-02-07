
CCPROTOCOMPILER=$(BINROOT)/thirdparty/protobuf/bin/protoc
CCPROTOCOMPILERLIBS=LD_LIBRARY_PATH=$(BINROOT)/thirdparty/protobuf/lib:$(BINROOT)/thirdparty/unixodbc/lib
DOXYGEN=$(BINROOT)/thirdparty/doxygen/bin/doxygen

CPPFLAGS=-I . -I $(OBJROOT) -I $(BINROOT)/thirdparty/protobuf/include -I $(BINROOT)/thirdparty/unixodbc/include -I $(BINROOT)/thirdparty/openssl/include -pipe -I $(BINROOT)/thirdparty/KeccakCodePackage/include -I $(BINROOT)/thirdparty/libgpg-error/include
CCFLAGS=$(CPPFLAGS) -m64 -Werror -Wall -g 
CFLAGS=$(CPPFLAGS) -m64 -Werror -Wall -g
LDFLAGS=
CCLDFLAGS=$(CCFLAGS) -L$(BINROOT)/thirdparty/protobuf/lib -L$(BINROOT)/thirdparty/openssl/lib -L$(BINROOT)/thirdparty/unixodbc/lib -L $(BINROOT)/thirdparty/KeccakCodePackage/lib
LD_RFLAG=-r -dead_strip -all_load
CCC=g++
CC=gcc
LIBS:=
ifeq ($(ARCH),Linux)
    VALGRIND=/usr/bin/valgrind --leak-check=yes --gen-suppressions=all --suppressions=textus/valgrind-suppressions --fullpath-after=src/
    LIBS:=-luuid
    BEGIN_LIBS = -Xlinker --start-group
    END_LIBS = -Xlinker --end-group
endif
LIBS:=$(LIBS) -lodbc -lpthread -lprotobuf -lssl -lcrypto -ldl -lkeccak
LIB_PATHS=$(BINROOT)/thirdparty/protobuf/lib:$(BINROOT)/thirdparty/openssl/lib:$(BINROOT)/thirdparty/unixodbc/lib
ifeq ($(ARCH),OS-X)
     AR=ar
     ARFLAGS=-r
     CCFLAGS:=$(CCFLAGS) -dead_strip -all_load
     CFLAGS:=$(CFLAGS) -dead_strip -all_load
else
     CCFLAGS:=$(CCFLAGS) -rdynamic
     CFLAGS:=$(CFLAGS) -rdynamic
     AR=ar
     ARFLAGS=-r -s
endif
