/* String.h -*- c++ -*-
 * Copyright (c) 2014 Ross
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
#ifndef TEXTUS_UTIL_STRING_H
#define TEXTUS_UTIL_STRING_H

#include <string>
#include <list>

#define XSTR(x) #x
#define STRING(x) XSTR(x)

namespace textus { namespace util {

using namespace std;

class StringUtils { // really just a namespace
public:
#define split_func(name, type, len, func)			 \
  static list<string> name(const string &p1, type p2) {	 \
    size_t b = 0;						 \
    list<string> res;						 \
    size_t l = len;						 \
    while (b != string::npos && b < p1.length()) {		 \
       size_t e = p1.func(p2, b);				 \
       if (e != string::npos) {					 \
         res.push_back(p1.substr(b, e-b));			 \
	 b = e + l;						 \
       } else {							 \
         res.push_back(p1.substr(b));				 \
	 b = e;							 \
       }							 \
    }								 \
    return res;							 \
  }								 \


  split_func(split, const string &, p2.length(), find)
  split_func(split, char , 1, find);
  split_func(splitAny, const string &, 1, find_first_of);
  split_func(splitOther, const string &, 1, find_first_not_of);

#undef split_func

  static string trim(const string &in, const string chars=" \t\r\n") {
    string out = in;
    while (out.length() > 0 && 
	   chars.find(out[out.length()-1]) != string::npos) {
      out = out.substr(0, out.length() - 1);
    }
    return out;
  }
  


};

}} //namespace

#endif //TEXTUS_UTIL_STRING_H
