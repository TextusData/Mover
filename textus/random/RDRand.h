/* RDRand.h -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
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
#include "textus/random/PRNG.h"

#ifndef TEXTUS_RANDOM_RDRAND_H_
#define TEXTUS_RANDOM_RDRAND_H_

namespace textus { namespace random {

class RDRand: public PRNG {
private:
  bool test_rdrand();
  bool rdrand_available;

protected:
  virtual uint64_t randombits(int bits);

public:
  explicit RDRand(): rdrand_available(false) {}
  virtual ~RDRand() {}

  virtual int seed(uint64_t sd) {
    rdrand_available = test_rdrand();
    if (!rdrand_available) {
      LOG(INFO) << "RDRAND not available.  Possible reduction in quality of random numvers.\n";
    }
    return 0;
  }

};

class RDRandFactory: public PRNGFactory {
 public:
  explicit RDRandFactory() {}
  virtual ~RDRandFactory() {}
  virtual PRNG* create(const std::string &name) { return new RDRand(); }

};

}} //namespace

#endif //TEXTUS_RANDOM_RDRAND_H_
