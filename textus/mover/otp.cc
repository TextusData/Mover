/* opt.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
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
