#
# This file was generated from libmd5.pro.in on Sun May 19 12:05:35 CEST 2013
#

TEMPLATE	= lib
CONFIG		= warn_on staticlib debug
HEADERS		= md5.h md5_loc.h
SOURCES		= md5.c
win32:INCLUDEPATH          += .
win32-g++:TMAKE_CFLAGS     += -D__CYGWIN__ -DALL_STATIC
DESTDIR                    =  ../lib
TARGET                     =  md5
OBJECTS_DIR                =  ../objects

TMAKE_MOC = /usr/bin/moc
LIBS += -L/usr/local/lib
INCLUDEPATH += /usr/local/include
