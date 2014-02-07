/* PRNG.cc -*- C++ -*-
 * Copyright (C) 2010, 2013 Ross Biro
 *
 * Represents a pseudo random number generator.
 *
 */

#include "textus/random/BBS.h"
#include "textus/file/FileHandle.h"
#include "textus/system/SysInfo.h"

#include <sys/time.h>

namespace textus { namespace random {

BBS::BBS() {
  struct timeval tv;
  volatile uint64_t uninitialized;
  pid_t p1 = getpid();
  pid_t p2 = getppid();
  gettimeofday(&tv, NULL);

  seedy = static_cast<uint64_t>(tv.tv_sec ^ p1) << 32 ^ (tv.tv_usec ^ uninitialized ^ p2);
  size_t state_size = sizeof(uint64_t);
  unsigned char *state = static_cast<unsigned char *>(alloca(state_size));
  textus::system::SysInfo::systemInformation()->random(state, state_size);
  seedy = seedy ^ *reinterpret_cast<uint64_t *>(state);
}

BBS::BBS(uint64_t s) {
  seedy = s;
}

int BBS::randombit() {
  seedy = (seedy * seedy) % M;
  return seedy & 1;
}

uint64_t BBS::randombits(int bits) {
  uint64_t a = 0;
  for (int i = 0; i < bits; i++) {
    a = (a << 1) | randombit();
  }
  return a;
}

int BBS::seed(uint64_t s) {
  seedy = s;
  return 0;
}

}} //namespace
