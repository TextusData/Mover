/* BitStirrer.h -*- c++ -*-
 * Copyright (c) Ross Biro 2013
 *
 * Use the Keccak hash to stir bits
 * for the random number generators.
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

#ifndef TEXTUS_HASH_BITSTIRRER_H_
#define TEXTUS_HASH_BITSTIRRER_H_

#include "textus/base/Base.h"

namespace textus { namespace hash {
extern "C" {
#include "KeccakDuplex.h"
}

class BitStirrer: virtual public textus::base::Base {
private:
  Keccak_DuplexInstance di;
  unsigned int rate;
  unsigned int capacity;

public:
  explicit BitStirrer(): rate(808), capacity(792) {}
  virtual ~BitStirrer() {}
  virtual int init() {
    Synchronized(this);
    int ret  = 0;
    HRC(Keccak_DuplexInitialize(&di, rate, capacity));
  error_out:
    return ret;
  }

  virtual int getBits(const unsigned char *in, size_t in_len,
		  unsigned char *out, size_t out_len) {
    int ret = 0;
    HRC(Keccak_Duplexing(&di, in, in_len, out, out_len, 1));
  error_out:
    return ret;
  }
};

}} // namespace

#endif //TEXTUS_HASH_BITSTIRRER_H_
