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

#ifndef TEXTUS_UTIL_HEX_H_
#define TEXTUS_UTIL_HEX_H_

#include <string>

namespace textus { namespace util {

std::string hex_encode(const unsigned char *, unsigned int);
std::string hex_decode(const std::string &);
static inline std::string hex_encode(std::string s) { 
  return hex_encode(reinterpret_cast<const unsigned char *>(s.c_str()), s.length()); 
}


}} //namespace

#endif // TEXTUS_UTIL_HEX_H_
