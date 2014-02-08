/* Hex.h -*- c++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 * Convert binary to and from ascii-hex
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

#include "textus/util/Hex.h"

namespace textus { namespace util {
using namespace std;

static inline char hexchar(unsigned char c) {
  return (c < 10)?(c + '0'):(c + 'A' - 10);
}

string hex_encode(const unsigned char *c, unsigned int l) {
  string s;
  s.reserve(2*l);
  for (unsigned i = 0; i < l; i++) {
    unsigned char n1 = c[i] >> 4;
    unsigned char n2 = c[i] & 0xf;
    s = s + hexchar(n1) + hexchar(n2);
  }
  return s;
}

static inline char nibblehex(char in) {
  if (in >= '0' && in <= '9') {
    return in - '0';
  }
  if (in >= 'a' && in <= 'f') {
    return in - 'a' + 10;
  }
  if (in >= 'A' && in <= 'f') {
    return in - 'A' + 10;
  }
  return -1;
}

static inline char charhex(char c1, char c2) {
  char c;
  c = (nibblehex(c1) << 4) + nibblehex(c2);
  return c;
}

string hex_decode(const std::string &in) {
  string o;
  unsigned int l = in.length();

  if (l & 1) {
    return o;
  }
  o.reserve(l/2);
  for (unsigned int i = 0; i < l; i += 2) {
    o = o + charhex(in[i], in[i+1]);
  }

  return o;
}

}} //namespace
