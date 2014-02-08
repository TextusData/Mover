/* RDTsc.h -*- c++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 * Read the tsc and rotate it to use it as
 * a source of entropy to feed into the
 * mixer.
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

#ifndef TEXTUS_RANDOM_RDTSC_H_
#define TEXTUS_RANDOM_RDTSC_H_

#include "textus/random/PRNG.h"

namespace textus { namespace random {

class RDTsc: virtual public PRNG {
protected:
  virtual uint64_t randombits(int bits) {
    static unsigned counter;
    uint64_t lowbits, highbits;
    volatile uint64_t tmp;
    uint64_t result = tmp;
    //Cause a context switch to try to get some entropy into the system.
    Thread::self()->yield();
    asm volatile("rdtsc" : "=a" (lowbits), "=d" (highbits));
    result = result ^ lowbits ^ highbits;
    // Racy, but that's another source of entropy.
    unsigned char rot = (++counter & 63);
    result = (result >> (64 - rot)) | (result << rot);
    return result;
  }

public:
  explicit RDTsc() {};
  virtual ~RDTsc() {};

};

class RDTscFactory: public PRNGFactory {
 public:
  explicit RDTscFactory() {}
  virtual ~RDTscFactory() {}
  virtual PRNG* create(const std::string &name) { return new RDTsc(); }

};


}} //namespace


#endif //TEXTUS_RANDOM_RDTSC_H_
