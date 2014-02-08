/* Hasher.h -*- c++ -*-
 * Copyright (c) Ross Biro 2013
 *
 * Encapsulate all the different avaiable hashes.
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

#ifndef TEXTUS_HASH_HASHER_H_
#define TEXTUS_HASH_HASHER_H_

#include "textus/base/Base.h"
#include "textus/random/Random.h"

#include <string>

namespace textus { namespace hash {
using namespace std;

class Hasher: virtual public textus::base::Base {
public:
  Hasher() {}
  virtual ~Hasher() {}
  virtual int init() = 0;
  virtual int finalize() = 0;
  virtual int addData(string data)=0;
  virtual string getHash() = 0;
  virtual string computeHash(string data) {
    int ret= 0 ;
    Synchronized(this);
    HRC(init());
    HRC(addData(data));
    HRC(finalize());
    return getHash();
  error_out:
    return string("");
  }
  virtual string randomHash() {
    unsigned char buff[64];
    textus::random::Random::randomize_buffer(buff, sizeof(buff));
    return string(reinterpret_cast<char *>(buff), sizeof(buff));
  }
};

}} //namespace

#endif // TEXTUS_HASH_HASHER_H_
