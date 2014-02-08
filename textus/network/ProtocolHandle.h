/* ProtocolHandle.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with networking protocols.
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

#ifndef TEXTUS_NETWORK_PROTOCOL_HANDLE_H_
#define TEXTUS_NETWORK_PROTOCOL_HANDLE_H_

namespace textus { namespace network {

using namespace textus::file;
using namespace std;

class ProtocolHandle: virtual public FileHandle {
private:
  AutoDeref<ProtocolSocket> ps;
  string buffer;

protected:
public:
  ProtocolHandle() {}
  virtual ~ProtocolHandle() {}

};

}} //namespace


#endif // TEXTUS_NETWORK_PROTOCOL_HANDLE_H_
