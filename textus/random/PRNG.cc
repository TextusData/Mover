/* PRNG.cc -*- C++ -*-
 * Copyright (C) 2010, 2013 Ross Biro
 *
 * Represents a pseudo random number generator.
 *
 */

#include "textus/random/PRNG.h"
#include "textus/random/Random.h"
#include "textus/file/FileHandle.h"
#include "textus/base/init/Init.h"
#include "textus/random/SysRand.h"
#include "textus/random/RDRand.h"
#include "textus/random/RDTsc.h"
#include "textus/random/Mixer.h"

#include <sys/time.h>
#include <fcntl.h>

namespace textus { namespace random {

DEFINE_STRING_ARG(prng_default_generator, "mixer", "prng-default-generator", "The default PRNG to use to extend the system random number generator.");

PRNG *PRNG::globalPRNG() {
  static Base lock;
  static PRNG *global;
  Synchronized(&lock);
  if (global == NULL) {
    global = Random::findGenerator(prng_default_generator);
    if (global == NULL) {
      LOG(ERROR) << "Unable to locate random number geneator: "
		 << prng_default_generator << " exiting.\n";
      die();
    }
  }
  return global;
}

PRNG::PRNG() {
}

DEFINE_INIT_FUNCTION(PRNG_init, EARLY_INIT_PRIORITY) {
  int ret = 0;
  HRC(Random::registerGenerator(string("rdrand"), new RDRandFactory()));
  HRC(Random::registerGenerator(string("mixer"), new MixerFactory()));
  HRC(Random::registerGenerator(string("rdtsc"), new RDTscFactory()));
  HRC(Random::registerGenerator(string("system"), new SysRandFactory()));
 error_out:
  return ret;
}


}} //namespace
