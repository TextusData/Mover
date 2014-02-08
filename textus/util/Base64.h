/* Base64.h -*- c++ -*-
 * Copyright 2013 Ross Biro
 *
 * Declrations for base64 functions.  See .cc file
 * for original copyright.
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

#ifndef TEXTUS_UTIL_BASE64_H_
#define TEXTUS_UTIL_BASE64_H_

namespace textus { namespace util {

std::string base64_encode(const unsigned char *, unsigned int);
std::string base64_decode(const std::string &);
static inline std::string base64_encode(std::string s) { 
  return base64_encode(reinterpret_cast<const unsigned char *>(s.c_str()), s.length()); 
}

}} //namespace

#endif // TEXTUS_UTIL_BASE64_H_
