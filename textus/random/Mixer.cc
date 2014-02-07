/* Mixer.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
 *
 */

#include "textus/random/Random.h"
#include "textus/random/Mixer.h"

namespace textus { namespace random {

using namespace textus::hash;

DEFINE_LIST_ARG(string, mixer_source_list, "mixer_source_list", "The list of random number generators to feed into the mixer.", "system, rdtsc, rdrand");
DEFINE_INT_ARG( mixer_random_seed_loops, 107, "mixer_random_seed_loops", "Number of times to run the mixer loop during the seed operation to get some entropy into the internal state.");

Mixer::~Mixer() {
  for (std::list<Source>::iterator i = sources.begin();
       i != sources.end(); ++i) {
    i->first->deref();
    i->first = NULL;
  }
  sources.clear();
}

int Mixer::seed(uint64_t sd) {
  int ret = 0;
  Synchronized(this);
  saved_seed = sd;
  if (!initialized) {
    stir = new BitStirrer();
    HRC(stir->init());
    for (std::list<std::string>::iterator i = mixer_source_list.begin(); 
	 i != mixer_source_list.end(); ++i) {
      std::string s = *i;
      size_t f = s.find_first_of('*');
      int count = 1;
      if (f != std::string::npos) {
	count = atoi(s.c_str());
	s = s.substr(f+1);
      }
      if (count == 0) {
	LOG(INFO) << "Skipping random source " << s
		  << " in mixer because count = 0" << "\n";
	continue;
      }
      PRNG *prng = Random::findGenerator(s);
      if (prng != NULL) {
	Source src;
	src.first = prng;
	src.second = count;
	sources.push_back(src);
	src.first->seed(sd);
      }
    }
    initialized = true;
    assert(sources.size() > 1);
  } else {
    for (std::list<Source>::iterator i = sources.begin();
	 i != sources.end(); ++i) {
      i->first->seed(sd);
    }
  }


  // Attempt to get some random internal state.
  for (int i = 0; i < mixer_random_seed_loops; ++i) {
    randombits(0);
  }

 error_out:
  return ret;
}

uint64_t Mixer::randombits(int bits) {
  int ret = 0;
  int in_bits   = ( bits == 0 ) ? 64 : bits;
  int in_count  = ( bits * 2 ) / 64 + 1;
  int out_count = ( bits == 0 ) ? 0 :( bits + 7 ) / 8;
  uint64_t *in;
  unsigned char out[sizeof(uint64_t)];

  assert(in_bits > 0 && in_bits <= 64);
  assert(sources.size() > 1);

  // uninitialized on stack.  Should produce a little bit of entropy.
  in = reinterpret_cast<uint64_t *>(alloca(sizeof(*in) * in_count));
  for (int k = 0; k < in_count; ++k) {
    in[k] ^= saved_seed;
    for (std::list<Source>::iterator i = sources.begin();
	 i != sources.end(); ++i) {
      for (int j = 0; j < i->second; ++j) {
	in[k] ^= i->first->rand64();
      }
    }
  }
  HRC(stir->getBits(reinterpret_cast<unsigned char *>(in), in_count*sizeof(*in),
		    out, out_count));

  uint64_t r;
  r = 0;
  for (int i = 0; i < out_count; ++i) {
    r = r << 8;
    r = r | out[i];
  }

  return r;

 error_out:
  LOG(ERROR) << "Unable to generate random numbers.  Quiting.\n";
  die();
  return 0;
}

}} // namespace
