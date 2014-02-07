/* Password.h -*- c++ -*-
 * Copyright (c) 2012
 *
 * Encapsualtes a password for better practices.
 */

#ifndef TEXTUS_BASE_PASSWORD_H_
#define TEXTUS_BASE_PASSWORD_H_

namespace textus { namespace base {

class Password  {
private:
  string pw;
public:
  Password() {}
  ~Password() {}

  void pin() {}
  void unpin() {}
  
  string::size_type length() const { return pw.length(); }
  const char *c_str() const { return pw.c_str(); }
};

}} // namespace

#endif // TEXTUS_BASE_PASSWORD_H_
