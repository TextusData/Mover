/* URL.cc -*- c++ -*-
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

#include "textus/network/URL.h"

namespace textus { namespace network {

URL *URL::parseURL(string url) {
  //should be of the form <alpha>*://<hostname>:port/path
  //If :// is missing, assume it's just a path.
  size_t eop = url.find("://");
  if (eop == string::npos) {
    URL *u = new URL();
    u->setproto(Protocol::defaultProtocol()); 
    eop = url.find("/");
    if (eop != string::npos) {
      u->setpath(url.substr(eop+1));
      url = url.substr(0, eop);
    }
    eop = url.find(":");
    if (eop != string::npos) {
      u->setport(atoi(url.substr(eop+1).c_str()));
      url = url.substr(0, eop);
    }
    if (url.length() == 0) {
      u->sethostname(string("localhost"));
    } else {
      u->sethostname(url);
    }
    return u;
  } 
  URL *u = new URL();
  if (u == NULL) {
    return NULL;
  }

  string protoname = url.substr(0, eop);
  u->setproto(Protocol::findProtocol(protoname));
  if (u->proto() == NULL) {
    return NULL;
  }

  size_t eoh = url.substr(eop+3).find('/');
  u->sethostname(url.substr(eop+3, eoh));
  if (eoh != string::npos) {
    u->setpath(url.substr(eop+3+eoh, string::npos));
  }
  eoh = u->hostname().rfind(':');
  if (eoh != string::npos) {
    u->setport(atoi(u->hostname().substr(eoh+1).c_str()));
    u->sethostname(u->hostname().substr(0, eoh));
  }
  return u;
}

NetworkAddress *URL::getAddress() {
  return NetworkAddress::resolve(hostname());
}

}} //namespacep
