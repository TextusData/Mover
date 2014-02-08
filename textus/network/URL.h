/* URL.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with URLs
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

#ifndef TEXTUS_NETWORK_URL_H_
#define TEXTUS_NETWORK_URL_H_

#include "textus/base/Base.h"
#include "textus/network/Protocol.h"
#include "textus/network/NetworkAddress.h"


namespace textus { namespace network {
using namespace std;

class URL: virtual public Base {
private:
  MVAR(public, Protocol, proto);
  MSTRING(public, hostname);
  MSTRING(public, path);
  MINT(public, port); // 0 = default -1 = random -2 = na
  
protected:
public:
  static URL *parseURL(string url);
  static URL *parseURL(const char *);

  URL():port_(0) {}
  virtual ~URL() {}
  
  NetworkAddress *getAddress();
  
};

}} //namespace


#endif // TEXTUS_NETWORK_URL_H_
