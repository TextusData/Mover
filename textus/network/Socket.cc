/* Socket.cc -*- c++ -*-
 * Copyright (c) 2009, 2013 Ross Biro
 *
 * Represents a network socket.
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
#include "textus/base/Base.h"
#include "textus/network/Socket.h"
#include "textus/network/URL.h"
#include "textus/system/SysInfo.h"

namespace textus { namespace network {

DEFINE_STRING_ARG(socks_user_id, "", "socks_user_id", "The userid to use when connecting to a socks server.");
DEFINE_BOOL_ARG(socket_use_socks, false, "socket_use_socks", "Set to turn on the use of sockets (defaults to true to use TOR.)");
DEFINE_STRING_ARG(socket_default_socks_server, "localhost:9050", "default_socks_server", "default url for socks server.");


static long buffer_size;
static long packets_in_buffer;
static long bytes_sent;

void SocketHelper::close() {
  Synchronized(this);
  parent()->removeWatcher(this);
  parent()->setHelper(next());
  set_parent(NULL);
}


class SocketSocksHelper: public SocketHelper {
  MVAR(public, URL, url);
  bool wrote;
  string red;
  bool non_blocking;

public:
  explicit SocketSocksHelper(Socket *p, URL *u): SocketHelper(p),
						 url_(u), wrote(false) {}
  virtual ~SocketSocksHelper() {}
  virtual void signalReceived() {
    Socket *sock = parent();
    if (sock->hadError()) {
      close();
      return;
    }

    if (!sock->isConnected()) {
      return;
    }

    if (!wrote) {
      sendSocksMessage(url());
      wrote = true;
    } else {
      // XXXXXXX Fixme: need to deal with partial reads.
      red = parent()->read(1024);
      int ret = processSocksMessage(red);
      if (ret < 0) {
	parent()->hadError();
      }
      close();
    }
  }

  int processSocksMessage(string mess) {
    size_t length = mess.length();
    const unsigned char *buff = 
      reinterpret_cast<const unsigned char *>(mess.c_str());
    if (length != 8) {
      return -1;
    }
    if (buff[0] != 0x04) {
      LOG(INFO) << "Got unknown socks return packet version: " <<
	(unsigned)buff[0] << "\n";
      return -1;
    }
    if (buff[1] != 0x90) {
      LOG(INFO) << "Got socks error: " << 
	(unsigned)buff[1] << "\n";
      return -1;
    }
    return 0;
  }

  int sendSocksMessage(URL *url) {
    int ret = 0;
    string userID; 
    unsigned char *buff;
    string domain_name;
    if (socks_user_id == "") {
      userID = textus::system::SysInfo::systemInformation()->getUser();
    } else {
      userID = socks_user_id;
    }
    domain_name = url->hostname();
    // from the socks 4a protocol:
    /*

		+----+----+----+----+----+----+----+----+----+----+....+----+
		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
		+----+----+----+----+----+----+----+----+----+----+....+----+
 # of bytes:	   1    1      2              4           variable       1

 So we get 8 bytes for the header + length of the two strings + the two null
 bytes.  This gives us 10 bytes + the length of the two strings. 
    */

    int len = 10 + domain_name.length() + userID.length();
    buff = static_cast<unsigned char *>(alloca(len));
    HRNULL(buff); // alloca can't really fail.

    buff[0] = 4; // version number.
    buff[1] = 1; // connect
    buff[2] = (url->port() & 0xff00) >> 8; // port in network byte order.
    buff[3] = (url->port() & 0xff);
    buff[4] = buff[5] = buff[6] = 0; // ip address 0.0.0.1
    buff[7] = 1;

    // the +1's make sure we get the null byte.
    strncpy(reinterpret_cast<char *>(buff + 8), userID.c_str(),
	    userID.length() + 1);
    strncpy(reinterpret_cast<char *>(buff + 9 + userID.length()),
	    domain_name.c_str(),
	    domain_name.length() + 1);

    buff[len - 1] = 0; // just to make sure.
    if (parent()->write(string(reinterpret_cast<char *>(buff), len))
	== len) {
      goto error_out;
    }
    ret = -errno;

  error_out:
    return ret;

  }
};

class SocketConnectHelper: public SocketAddressHelper {
public:
  SocketConnectHelper(Socket *s, NetworkAddress *na):SocketAddressHelper(s, na) {}
  virtual ~SocketConnectHelper() {}
  virtual void signalReceived() {
    Synchronized(this);
    if (na && na->isValid()) {
      parent()->connect(na);
      close();
    }
  }
};

int Socket::connect(URL *url) {
  AUTODEREF(SocketHelper *, sch);
  int ret = 0;
  Synchronized(this);
  if (socket_use_socks) {
    bool non_blocking = nonBlocking();
    sch = new SocketSocksHelper(this, url);
    connect_helper = sch;
    addWatcher(sch);
    setNonBlocking(true);
    HRC(connect(getSocksServer()));
    if (!non_blocking) {
      while(connect_helper != NULL) {
	wait();
      }
      setNonBlocking(false);
    }
  } else {
    if (url->port()) {
      port = url->port();
    }

    NetworkAddress *na = url->getAddress();
    HRNULL(na);
    sch = new SocketConnectHelper(this, na);
    na->addWatcher(sch);
    connect_helper = sch;
  }
  error_out:
    return ret;
}

NetworkAddress *Socket::getSocksServer() {
  static URL *url;
  static Base lock;
  Synchronized(&lock);
  if (url != NULL) {
    return url->getAddress();
  }
  url = URL::parseURL(socket_default_socks_server);
  if (url == NULL) {
    return NULL;
  }
  return url->getAddress();
}

void Socket::sendData(string data) {
  Synchronized(this);
  buffer_size += data.length();
  packets_in_buffer++;
  buffer.push_back(data);
  spaceAvailable();
}
  
void Socket::spaceAvailable() {
  Synchronized(this);
  while (!buffer.empty()) {
    string data = buffer.front();
    buffer.pop_front();
    int wrote = write(data);
    if (wrote < 0) {
      wrote = 0;
      int e = errno;
      LOG(DEBUG) << "Write error errno=" << e << "\n";
    }

    buffer_size -= wrote;
    bytes_sent += wrote;
    packets_in_buffer --;
    if ((unsigned)wrote == data.length()) {
      continue;
    }

    data = data.substr(wrote);
    buffer.push_front(data);
    packets_in_buffer ++;

    watch_write = true;

    return;
  }
  watch_write = false;
  FileHandle::spaceAvailable();
}

void Socket::error(textus::event::Event *e) {
  SocketEvent *se = dynamic_cast<SocketEvent *>(e);
  if (se != NULL) {
    LOG(ERROR) << "Socket Error: " << se->error() << "\n";
  }
  FileHandle::error();
}

textus::event::Event *SocketEventFactory::errorEvent(Error *e) {
  SocketEvent *se = new SocketEvent(socket);
  se->setError(e);
  setEventQueue(se);
  return se;
}

textus::event::Event *SocketEventFactory::readEvent(string s) {
  SocketEvent *se = new SocketEvent(socket);
  se->setData(s);
  setEventQueue(se);
  return se;
}

textus::event::Event *SocketEventFactory::writeEvent() {
  SocketEvent *se = new SocketEvent(socket);
  se->setWrite(true);
  setEventQueue(se);
  return se;
}

void Socket::read(SocketEvent *se) {
  Synchronized(this);
  data = data + se->data();
}


}} // namespace
