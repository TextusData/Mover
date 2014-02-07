/* opt.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 */
#include <stdio.h>

#include <string>

#include "textus/base/Base.h"
#include "textus/base/init/Init.h"
#include "textus/random/Random.h"

DEFINE_INT_ARG(otp_generate, 1024, "generate",
	       "Number of bits to generate.  Must be multiple of 8.");


int main(int argc, const char *argv[], const char *envp[]) {
  int ret = 0;
  textus::base::init::TextusInit(argc, argv, envp);
  // We have to seed the PRNG.  Several use this
  // to initialize things rather than actually seed.
  textus::random::PRNG::globalPRNG()->seed(time(NULL));
  std::string s = textus::random::Random::data(otp_generate/8, otp_generate/8);
  write(1, s.c_str(), s.length()); ;
  fsync(1);
  textus::base::init::TextusShutdown();
  return ret;
}
