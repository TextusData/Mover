/* FileProtocol.h -*- c++ -*-
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

#ifndef TEXTUS_NETWORK_PROTCOLS_FILE_PROTOCOL_H_
#define TEXTUS_NETWORK_PROTCOLS_FILE_PROTOCOL_H_

namespace textus { namespace network { namespace protcols {

class FileProtocol: virtual public Protocol {
private:

public:
  FileProtocol() {}
  virtual ~FileProtocol() {}
  virtual ProtocolSocket *connect(string hostname);
  virtual ProcotolServer *listen() {
    return NULL;
  }

};

}}} //namespace

#endif // TEXTUS_NETWORK_PROTCOLS_FILE_PROTOCOL_H_
