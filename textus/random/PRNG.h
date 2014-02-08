/* PRNG.h -*- C++ -*-
 * Copyright (C) 2010, 2013, 2014 Ross Biro
 *
 * Represents a pseudo random number generator.
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

#ifndef TEXTUS_RANDOM_PRNG_H_
#define TEXTUS_RANDOM_PRNG_H_

#include "textus/base/Base.h"

namespace textus { namespace random {

class PRNG: virtual public textus::base::Base {
private:
protected:
  // returns bits random bits in the low order bits of the result.
  virtual uint64_t randombits(int bits)=0;

public:
  static PRNG *globalPRNG();
  explicit PRNG();
  virtual ~PRNG() {}
  virtual int seed() {
    return seed(time(NULL) ^ getpid());
  }
  virtual int seed(uint64_t s) { return 0; }
  uint64_t rand64(void) { return randombits(64); }
  uint32_t rand32(void) { return randombits(32); }
  uint16_t rand16(void) { return randombits(16); }
  uint8_t  rand8(void)  { return randombits(8);  }
  int rand(int s) { return rand32() % s; }
};

class PRNGFactory: virtual public textus::base::Base {
public:
  explicit PRNGFactory() {}
  virtual ~PRNGFactory() {}
  virtual PRNG* create(const std::string &name)=0;
};

}} //namespace

#endif //TEXTUS_RANDOM_PRNG_H_
