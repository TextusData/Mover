/* Base64.h -*- c++ -*-
 * Copyright 2013 Ross Biro
 *
 * Declrations for base64 functions.  See .cc file
 * for original copyright.
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
