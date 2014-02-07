/* FileBackedNumber.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 */
#include <stdlib.h>

#include "textus/file/FileBackedNumber.h"
#include "textus/file/TextusFile.h"

namespace textus { namespace file {

uint64_t FileBackedNumber::get() {
  Synchronized(this);
  if (lock) {
    lockFile();
  }
  fh->seek(0, SEEK_SET);
  string s = fh->read(1024);
  long long n = atoll(s.c_str());
  if (lock) {
    unlockFile();
  }
  return n;
}

uint64_t FileBackedNumber::increment(uint64_t i) {
  unsigned long long res;
  Synchronized(this);
  if (lock) {
    lockFile();
  }
  fh->seek(0, SEEK_SET);
  string s = fh->read(1024);
  long long n = atoll(s.c_str());
  res = n + i;
  fh->seek(0, SEEK_SET);
  char buff[60];
  snprintf(buff, sizeof(buff), "%llu", res);
  fh->write(buff);
  fh->flush();
  if (lock) {
    unlockFile();
  }

  return n;
  
}

void FileBackedNumber::set(uint64_t n) {
  Synchronized(this);
  if (lock) {
    lockFile();
  }
  fh->seek(0, SEEK_SET);
  char buff[60];
  snprintf(buff, sizeof(buff), "%llu", (long long unsigned)n);
  fh->write(buff);
  fh->flush();
  if (lock) {
    unlockFile();
  }
}

}} //namespace
