/* Password.h -*- c++ -*-
 * Copyright (c) 2012
 *
 * Encapsualtes a password for better practices.
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
