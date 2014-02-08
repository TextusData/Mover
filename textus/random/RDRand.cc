/* RDRand.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * See rdrand.h for important warning.
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

#include "textus/random/RDRand.h"
#include "textus/base/SignalHandler.h"

namespace textus { namespace random {
 
/* borrowed from the linux kernel. */
#define RDRAND_LONG	".byte 0x48,0x0f,0xc7,0xf0"

static int rdrand(uint64_t *rand_out, int retry_loops = 30 /* arbitrary. */) {
	int ok=0;
	__asm__ volatile("1: " RDRAND_LONG "\n\t"
			 "jc 2f\n\t"
			 "decl %0\n\t"
			 "jnz 1b\n\t"
			 "2:"
			 : "=r" (ok), "=a" (*rand_out)
			 : "0" (retry_loops));
	return ok;
}

/* End of borrowed code. */

bool RDRand::test_rdrand() {
  textus::base::SignalHandler sh;
  uint64_t ignored;
  sh.setsignal(SIGILL);
  sh.setaction(textus::base::SignalHandler::IGNORE);
  (void)rdrand(&ignored, 1);
  return sh.signals_caught() > 0;
}

uint64_t RDRand::randombits(int bits) {
  volatile uint64_t uninitialized;
  uint64_t results = uninitialized;
  uint64_t raw;
  uint64_t mask = (1 << bits) - 1;
  int ret=0;
  HRZ(rdrand(&raw));
  if (bits == 64) {
    results = results ^ raw;
    goto out;
  }

  while (raw != 0) {
    results = results ^ (raw & mask);
    raw = raw >> bits;
  }
  results = results & mask;

  goto out;

 error_out:
  LOG(ERROR) << "RDRand unable to generate random numbers.\n";
  die();
 out:
  return results;
}

}} //namespace
