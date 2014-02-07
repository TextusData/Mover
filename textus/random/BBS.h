/* PRNG.h -*- C++ -*-
 * Copyright (C) 2010, 2013 Ross Biro
 *
 * Represents a pseudo random number generator.
 *
 */

#ifndef TEXTUS_RANDOM_BBS_H_
#define TEXTUS_RANDOM_BBS_H_

#include "textus/base/Base.h"
#include "textus/random/PRNG.h"

namespace textus { namespace random {

class BBS: public PRNG {
private:
  // Attempt at Blum Blum Shub
  static const uint64_t M = (uint64_t)596572387 * (uint64_t)786431;
  uint64_t seedy;

  int randombit();
  virtual uint64_t randombits(int bits);

public:
  BBS();
  explicit BBS(uint64_t s);
  virtual ~BBS() {}
  int seed(uint64_t s);
};

}} //namespace

#endif //TEXTUS_RANDOM_BBS_H_
