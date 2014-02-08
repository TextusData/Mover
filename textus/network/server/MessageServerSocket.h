/* MessageServerSocket.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Class for a simple message oriented server.
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

#ifndef TEXTUS_NETWORK_SERVER_MESSAGE_SERVER_SOCKET_H_
#define TEXTUS_NETWORK_SERVER_MESSAGE_SERVER_SOCKET_H_

#include "textus/base/Base.h"
#include "textus/network/Socket.h"
#include "textus/file/ProtoBufReaderEvent.h"
#include "textus/network/server/ServerTemplate.h"

namespace textus { namespace network { namespace server {

template <class T> class MessageServerSocket: 
    public textus::network::server::ServerSocket<T, textus::network::Socket, textus::file::ProtoBufReader> 
{
public:
  explicit MessageServerSocket() {
    ServerSocket<T, Socket, ProtoBufReader>::lr = new ProtoBufReader(this);
    ServerSocket<T, Socket, ProtoBufReader>::lr->deref();
    
    AUTODEREF(textus::file::ProtoBufReader::EventFactoryType *, lref);
    lref = new textus::file::ProtoBufReader::EventFactoryType(ServerSocket<T, Socket, ProtoBufReader>::lr);
    ServerSocket<T, Socket, ProtoBufReader>::lr->setEventFactory(lref);

    ServerSocket<T, Socket, ProtoBufReader>::processor = new T(this);
    ServerSocket<T, Socket, ProtoBufReader>::processor->deref();
  }

  virtual ~MessageServerSocket() {}
};

template <class T> class MessageServerFactory: public SocketFactory
{
public:
  MessageServerFactory() {}
  virtual ~MessageServerFactory() {}
  virtual Socket *createSocket()
  {
    MessageServerSocket<T> *lss = new MessageServerSocket<T>();
    return lss;
  }

};

}}} // namespace

#endif //TEXTUS_NETWORK_SERVER_MESSAGE_SERVER_SOCKET_H_
