/* Hex.h -*- c++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 * Convert binary to and from ascii-hex
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
