/* Random.cc -*- c++ -*-
 * Copyright(c) 2013 Ross Biro
 *
 * Implementations of the random data generators.
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

#include "textus/base/Base.h"
#include "textus/base/init/Init.h"
#include "textus/random/Random.h"
#include "textus/random/PRNG.h"
#include "textus/system/SysInfo.h"

namespace textus { namespace random {

DEFINE_BOOL_ARG(random_use_prng, true, "random_use_prng", "If set, causes random numbers to be generated with a combination of /dev/random and a PRNG. (default true)");

ReferenceValueMap<string, PRNGFactory *> Random::generators;

static PRNG *prng=NULL;

static int initializer() {
  static textus::base::Base b;
  Synchronized(&b);
  if (random_use_prng) {
    prng = PRNG::globalPRNG();
  }

  return 1;
}

// XXXXXXX ALERT: This function *MUST* produce cryptographically strong
// random numbers.  Any weakness here will lead to the whole system
// being compromised.
void Random::randomize_buffer(unsigned char *buff, size_t len) {
  static int initialized=initializer();
  assert(initialized != 0);
  
  // Get something random from /dev/u?random.
  textus::system::SysInfo::systemInformation()->random(buff, len);
  if (prng != NULL) {
    for (size_t i = 0; i < len; ++i) {
      if (i == 512) {
	LOG(DEBUG) << "512 breakpoint.\n";
      }
      buff[i] ^= prng->rand8();
    }
  }
}

string Random::data(size_t min_len, size_t max_len) {
  int len = max_len == min_len?min_len:rand(max_len - min_len) + min_len;
  char *buff = static_cast<char *>(alloca(len));
  randomize_buffer(reinterpret_cast<unsigned char *>(buff), len);
  return string(buff, len);
}


string Random::head_pad() {
  unsigned char buff[8];
  buff[0] = rand8();
  size_t len = buff[0] & 0x7;
  if (len != 0) {
    randomize_buffer(buff+1, len);
  }
  return string(reinterpret_cast<char *>(buff), len+1);
}

uint64_t Random::rand64(void) {
  uint64_t r;
  randomize_buffer(reinterpret_cast<unsigned char *>(&r), sizeof(r));
  return r;
}

uint32_t Random::rand32(void) {
  uint32_t r;
  randomize_buffer(reinterpret_cast<unsigned char *>(&r), sizeof(r));
  return r;
}

uint16_t Random::rand16(void) {
  uint16_t r;
  randomize_buffer(reinterpret_cast<unsigned char *>(&r), sizeof(r));
  return r;
}

uint8_t Random::rand8(void) {
  uint8_t r;
  randomize_buffer(reinterpret_cast<unsigned char *>(&r), sizeof(r));
  return r;
}


}} //namespace
