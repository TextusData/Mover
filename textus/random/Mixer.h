/* Mixer.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Mix together a bunch of entropy sources with
 * Keccak (sha3).  This should at least obscure
 * any true lack of randomness in the inputs,
 * but cannot add any entropy to the system.
 */

#ifndef TEXTUS_RANDOM_MIXER_H_
#define TEXTUS_RANDOM_MIXER_H_

#include "textus/base/Base.h"
#include "textus/random/PRNG.h"
#include "textus/hash/BitStirrer.h"
#include "textus/base/AutoDeref.h"

#include <list>

namespace textus { namespace random {

using namespace textus::hash;

class Mixer:public PRNG {
private:
  typedef std::pair<PRNG *, int> Source;

  uint64_t saved_seed;
  textus::base::AutoDeref<BitStirrer> stir;
  std::list<Source> sources;
  bool initialized;

protected:
  virtual uint64_t randombits(int bits);

public:
  Mixer():initialized(false) {}
  virtual ~Mixer();
  
  virtual int seed(uint64_t s);

};

class MixerFactory: public PRNGFactory {
 public:
  explicit MixerFactory() {}
  virtual ~MixerFactory() {}
  virtual PRNG* create(const std::string &name) { return new Mixer(); }

};

}} //namespace


#endif //TEXTUS_RANDOM_MIXER_H_
