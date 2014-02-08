/* RDRand.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
 *
 * See rdrand.h for important warning.
 *
 */

#include "textus/random/RDRand.h"
#include "textus/base/SignalHandler.h"
#include "textus/system/SysInfo.h"

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
  return textus::system::SysInfo::systemInformation()->getCPUFlag("rdrand");

}

uint64_t RDRand::randombits(int bits) {
  volatile uint64_t uninitialized;
  uint64_t results = uninitialized;
  uint64_t raw;
  uint64_t mask = (1 << bits) - 1;
  int ret=0;
  if (!rdrand_available) {
    return raw;
  }
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
