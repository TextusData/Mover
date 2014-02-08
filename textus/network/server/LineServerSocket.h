/* LineServerSocket.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Class for a simple line oriented server.
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

#ifndef TEXTUS_NETWORK_SERVER_LINE_SERVER_SOCKET_H_
#define TEXTUS_NETWORK_SERVER_LINE_SERVER_SOCKET_H_

#include "textus/base/Base.h"
#include "textus/network/Socket.h"
#include "textus/file/LineReaderEvent.h"
#include "textus/network/server/ServerTemplate.h"

namespace textus { namespace network { namespace server {

template <class T> class LineServerSocket;

template <class T> class LineReaderServer: public textus::file::LineReader {
private:
public:
  explicit LineReaderServer(LineServerSocket<T> *s);
  virtual ~LineReaderServer() {}

  void lineRead(LineReaderEvent *lre);
  void eof(LineReaderEvent *lre);
};

template <class T> LineReaderServer<T>::LineReaderServer(LineServerSocket<T> *s): LineReader(s) 
{
}

template <class T> void LineReaderServer<T>::lineRead(LineReaderEvent *lre)
{
  Synchronized(this);
  LineServerSocket<T> *sock = dynamic_cast<LineServerSocket<T> *>(handle());
  LineReaderStatus::status stat;
  do {
    string s = readLine(stat);
    if (stat != LineReaderStatus::ok) {
      break;
    }
    sock->processMessage(s);
  } while (stat == LineReaderStatus::ok);
}

template <class T> void LineReaderServer<T>::eof(LineReaderEvent *lre)
{
  Synchronized(this);
  LineServerSocket<T> *s = dynamic_cast<LineServerSocket<T> *>(handle());
  s->eof(lre);
}

template <class T> class LineServerSocket: 
    public textus::network::server::ServerSocket<T, textus::network::Socket, LineReaderServer<T> > 
{
public:
  explicit LineServerSocket() {
    ServerSocket<T, Socket, LineReaderServer<T> >::lr = new LineReaderServer<T>(this);
    ServerSocket<T, Socket, LineReaderServer<T> >::lr->deref();

    AUTODEREF(textus::file::LineReader::EventFactoryType *, lref);
    lref = new textus::file::LineReader::EventFactoryType(ServerSocket<T, Socket, LineReaderServer<T> >::lr);
    ServerSocket<T, Socket, LineReaderServer<T> >::lr->setEventFactory(lref);

    ServerSocket<T, Socket, LineReaderServer<T> >::processor = new T(this);
    ServerSocket<T, Socket, LineReaderServer<T> >::processor->deref();
  }

  virtual ~LineServerSocket() {}
};

template <class T> class LineServerFactory: public SocketFactory
{
public:
  LineServerFactory() {}
  virtual ~LineServerFactory() {}
  virtual Socket *createSocket()
  {
    LineServerSocket<T> *lss = new LineServerSocket<T>();
    return lss;
  }

};

}}} // namespace

#endif //TEXTUS_NETWORK_SERVER_LINE_SERVER_SOCKET_H_
