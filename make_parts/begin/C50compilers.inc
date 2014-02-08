#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, version 3 of the
#   License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see
#   <http://www.gnu.org/licenses/>.
#
#
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
