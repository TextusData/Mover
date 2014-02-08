/* Random.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * contains the Random data generators.
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


#ifndef TEXTUS_RANDOM_RANDOM_H_
#define TEXTUS_RANDOM_RANDOM_H_
#include <string>

#include "textus/base/Base.h"
#include "textus/base/Range.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/random/PRNG.h"

namespace textus { namespace random {

using namespace std;
using namespace textus::base;


// Random class is really a namenspace for now.
class Random {
private:
  static ReferenceValueMap<string, PRNGFactory *> generators;
  
public:
  static void randomize_buffer(unsigned char *buff, size_t len);
  static string data(size_t min_len = 0, size_t max_len = 4096);
  static string head_pad();
  static uint64_t rand64(void);
  static uint32_t rand32(void);
  static uint16_t rand16(void);
  static uint8_t  rand8(void);
  static int rand(int s) { return rand32() % s; }
  static int registerGenerator(string name, PRNGFactory *f) {
    generators[name] = f;
    return 0;
  }
  static PRNG *findGenerator(std::string name) {
    if (generators.count(name) > 0) {
      return generators[name]->create(name);
    }
    return NULL;
  }

  template <typename I> static typename I::value_type* choose(const I &in, size_t k) {
    typename I::value_type *out=NULL;
    size_t count = 0;
    out = new typename I::value_type [k];
    if (!out) {
      return NULL;
    }

    for (typename I::iterator i = in.begin(); i != in.end(); ++i) {
      if (count < k) {
	out[count] = *i;
	count++;
      } else {
	count ++;
	unsigned r = rand(count);
	if (r < k) {
	  r = rand(k);
	  out[r] = *i;
	}
      }
    }
    return out;
  }

  template <typename I> static typename I::value_type* choose(I &in, size_t k) {
    typename I::value_type *out=NULL;
    size_t count = 0;
    out = new typename I::value_type [k];
    if (!out) {
      return NULL;
    }

    for (typename I::iterator i = in.begin(); i != in.end(); ++i) {
      if (count < k) {
	out[count] = *i;
	count++;
      } else {
	count ++;
	unsigned r = rand(count);
	if (r < k) {
	  r = rand(k);
	  out[r] = *i;
	}
      }
    }
    return out;
  }

  template <typename I> static typename I::value_type choose(const I &in) {
    typename I::value_type out;
    typename I::value_type *outp;

    outp = choose<I>(in, 1);

    out = outp[0];
    delete [] outp;
    return out;
  }

  int rand(Range<int, atoi> r) {
    int l = r.length();
    return rand(l) + r.min();
  }
}; 

}} //namespace


#endif // TEXTUS_RANDOM_RANDOM_H_
