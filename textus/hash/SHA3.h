/* SHA3.h -*- c++ -*-
 * Copyright (c) Ross Biro 2013
 *
 * Encapsulates Keccak hash.
 *
 */

#ifndef TEXTUS_HASH_SHA3_H_
#define TEXTUS_HASH_SHA3_H_

#include "textus/hash/Hasher.h"

namespace textus { namespace hash {

extern "C" {
#include "KeccakHash.h"
}

class SHA3: public Hasher {
  Keccak_HashInstance hi;
  unsigned int rate;
  unsigned int capacity;
  unsigned int hashbitlen;
  // always 1 since we only do whole bytes.
  unsigned char deliminatedSuffix;

public:
  // Default values for rate cand capacity were chosen via http://keccak.noekeon.org/tune.html for 
  // 2^128 collision resistance and 2^256 second preimage resistance.
  // This seems to be the best we can do with 256 bit output.
  explicit SHA3(): rate(1088), capacity(512), hashbitlen(256), deliminatedSuffix(1) {}
  virtual ~SHA3() {}
  virtual int init() {
    HashReturn hr;
    Synchronized(this);
    hr = Keccak_HashInitialize(&hi, rate, capacity, 0, deliminatedSuffix);
    if (hr == SUCCESS) {
      return 0;
    }
    return -1;
  }

  // Must be called before init.
  int setrc(unsigned int r, unsigned int c) {
    if ((r & 0x7) != 0 || r + c != 1600) {
      return -1;
    }
    Synchronized(this);
    rate = r;
    capacity = c;
    return 0;
  }

  // Interestingly enough, this can be called
  // at anytime.
  int setHashBitLen(int bits) {
    if ((bits & 0x7) != 0) {
      return -1;
    }
    Synchronized(this);
    hashbitlen = bits;
    return 0;
  }
  
  virtual int addData(string d) {
    HashReturn hr;
    Synchronized(this);
    hr = Keccak_HashUpdate(&hi, reinterpret_cast<const BitSequence *>(d.c_str()), 8 * d.length());
    if (hr == SUCCESS) {
      return 0;
    }
    return -1;
  }

  virtual int finalize() {
    HashReturn hr;
    Synchronized(this);
    hr = Keccak_HashFinal(&hi, NULL);
    if (hr == SUCCESS) {
      return 0;
    }
    return -1;
  }

  virtual string getHash() {
    HashReturn hr;
    unsigned char *buff;
    Synchronized(this);
    buff = static_cast<unsigned char *>(alloca(hashbitlen/8+1));
    if (buff == NULL) {
      goto error_out;
    }
    hr = Keccak_HashSqueeze(&hi, buff, hashbitlen);
    if (hr == SUCCESS) {
      return string(reinterpret_cast<char *>(buff), hashbitlen/8);
    }
  error_out:
    return string("");
  }

};

}} //namespace

#endif // TEXTUS_HASH_SHA3_H_
