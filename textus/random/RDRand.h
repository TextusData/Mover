/* RDRand.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Attempt to use RDRand instruction to generate
 * some entropy. 
 *
 * **** WARNING **** there is no way
 * to know if this is real entropy, or an encrypted
 * sequence that only some three letter government agency
 * knows.
 *
 * Further there has been some research that says the chip
 * can be sabotaged at the manufacturing level so that you
 * get substantially less entropy than you should, and it would
 * be almost impossible to detect.  The researchers were able to 
 * reduce the entropy to 32 bits and still have the rdrand
 * instruction pass all the standard tests.
 *
 * It seems unlikely that both techniques would be used at once,
 * since they likely would step on each others foot and then
 * at least one side would know what the other was up to.
 *
 * Using rdrand with the keckacck function shouldn't decrease
 * the entropy, but there is no guarantee it will increase it against
 * any particular advisory.
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
#include "textus/random/PRNG.h"

#ifndef TEXTUS_RANDOM_RDRAND_H_
#define TEXTUS_RANDOM_RDRAND_H_

namespace textus { namespace random {

class RDRand: public PRNG {
private:
  bool test_rdrand();

protected:
  virtual uint64_t randombits(int bits);

 public:
  explicit RDRand() {}
  virtual ~RDRand() {}
};

class RDRandFactory: public PRNGFactory {
 public:
  explicit RDRandFactory() {}
  virtual ~RDRandFactory() {}
  virtual PRNG* create(const std::string &name) { return new RDRand(); }

};

}} //namespace

#endif //TEXTUS_RANDOM_RDRAND_H_
