/* FileProtocol.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with files as a network protocol.
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

#include "textus/network/protocols/FileProtocol.h"

namespace textus { namespace network { namespace protocols {

ProtocolSocket *FileProtocol::connect(string hostname, int port) {
  if (hostname.length() != 0 || port != 0) {
    return NULL;
  }

  ProtocolSocket *ps = new ProtocolSocket();
  ps->setProtocol(this);
  return ps;
}



}}} //namespace
