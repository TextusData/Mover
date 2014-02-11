/* Socket.h -*- c++ -*-
 * Copyright (c) 2009-2010, 2013, 2014 Ross Biro
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

#ifndef TEXTUS_NETWORK_SOCKET_H_
#define TEXTUS_NETWORK_SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <list>

#include "textus/file/FileHandle.h"
#include "textus/network/NetworkAddress.h"
#include "textus/base/Error.h"
#include "textus/logging/Log.h"
#include "textus/base/init/Init.h"

#define NETWORK_INIT_PRIORITY 150

namespace textus { namespace network {
using namespace std;
using namespace textus::base;

class URL;
class Socket;
class SocketEvent;
class NetworkAddress;

DECLARE_BOOL_ARG(socket_use_socks);

class SocketHelper: virtual public textus::event::EventWatcher {
private:
  MVAR(public, SocketHelper, next); // last on first off
  MVAR(public, Socket, parent);
  MVAR(public, textus::file::FileHandleEventFactory, old_factory);

public:
  explicit SocketHelper(Socket *p):  parent_(p) {}
  virtual ~SocketHelper() {}
  virtual void close();
  virtual void watch();
};

class SocketAddressHelper: public SocketHelper {
protected:
  AutoDeref<NetworkAddress> na;
public:
  SocketAddressHelper(Socket *s, NetworkAddress *n):SocketHelper(s), na(n) {};
  virtual ~SocketAddressHelper() {}
  virtual void close() {
    Synchronized(this);
    if (na != NULL) {
      na->removeWatcher(this);
      na = NULL;
    }
    SocketHelper::close();
  }

};

class Socket: public textus::file::FileHandle {
private:
  list<string> buffer;
  AutoWeakDeref<Socket> parent;
  AutoDeref<NetworkAddress> peer;
  string data;
  int port;

protected:
  AutoDeref<SocketHelper> connect_helper;
  bool connected;

  virtual int connect(int fd, const struct sockaddr *sa, socklen_t sl) {
    return ::connect(fd, sa, sl);
  }

  virtual int read(int fd, void *buff, size_t buff_size) {
    return FileHandle::read(fd, buff, buff_size);
  }

public:
  Socket(): parent(NULL), peer(NULL), data(""), port(0), connected(false) {
    setNonBlocking(true);
  }

  virtual ~Socket() {
    Socket *p = parent;
    if (p) {
      setParentSocket(NULL);
      p->childDead(this);
    }
  }

  // needed by socket helpers.
  void watchRead() {
    Synchronized(this); 
    watch_write = false;
    watch_read = true;
    wakeMonitor();
  }

  void watchWrite() {
    Synchronized(this); 
    watch_write = true;
    watch_read = false;
    wakeMonitor();
  }

  void setHelper(SocketHelper *h) {
    Synchronized(this);
    connect_helper = h;
    signal();
  }

  int bind (NetworkAddress *na) {
    Synchronized(this);
    if (na) {
      socklen_t saddr_len=0;
      const sockaddr *sa = na->sockAddress(&saddr_len);
      return ::bind(nativeHandle(), sa, saddr_len);
    } else {
      struct sockaddr_in saddr;
      socklen_t saddr_len = INET_ADDRSTRLEN;
      memset(&saddr, 0, sizeof(saddr));
#ifdef HAVE_SIN_LEN
      saddr.sin_len = saddr_len;
#endif
      saddr.sin_family = AF_INET;
      return ::bind(nativeHandle(), reinterpret_cast<sockaddr *>(&saddr), saddr_len);
    }
  }

  void sendData(string data);

  void spaceAvailable();

  virtual void childDead(Socket *child) {
    return;
  }

  virtual int weakDelete() {
    {
      Synchronized(this);
      setEventFactory(NULL);
      Socket *p = parent;
      if (p) {
	setParentSocket(NULL);
	p->childDead(this);
      }
      while (connect_helper != NULL) {
	connect_helper->close();
      }
    }
    return FileHandle::weakDelete();
  }

  const Socket *parentSocket() {
    Synchronized(this);
    return parent;
  }

  void setParentSocket(Socket *s) {
    Synchronized(this);
    assert(parent != this);
    assert(s != this);
    parent = s;
  }

  int listen() {
    Synchronized(this);
    return ::listen(nativeHandle(), 256);
  }

  const NetworkAddress *peerAddress() {
    Synchronized(this);
    return peer;
  }

  void setPeerAddress(NetworkAddress *na) {
    peer = na;
  }

  NetworkAddress *getAddress() {
    struct sockaddr saddr;
    socklen_t saddr_len = sizeof(saddr);
    if (::getsockname(nativeHandle(), &saddr, &saddr_len))
      return NULL;
    return new NetworkAddress(saddr, saddr_len);
  }

  NetworkAddress *getPeerAddress() {
    struct sockaddr saddr;
    socklen_t saddr_len = sizeof(saddr);

    // This will give the name of the socks server if we are using
    // socks.  Not that useful.
    if (::getpeername(nativeHandle(), &saddr, &saddr_len)) {
      LOG(ERROR) << "Socket::getPeerAddress failed: " << errno << std::endl;
      return NULL;
    }
    return new NetworkAddress(saddr, saddr_len);
  }

  virtual int connect(URL *url);

  virtual int connect(NetworkAddress *addr) {
    socklen_t saddr_len;
    const sockaddr *sa;
    int ret = 0;
    bool add = false;

    {
      Synchronized(this);
      if (port != 0) {
	addr->setPort(htons(port));
      }
      sa = addr->sockAddress(&saddr_len);

      int fh = nativeHandle();

      if (fh < 0) {
	EXEC_BARRIER();
	fh = socket(sa->sa_family, SOCK_STREAM, 0);
	add = true;
	open(fh);
      }

      ret = connect(fh, sa, saddr_len);
      if (ret) {
	if (errno != EINPROGRESS && errno != EAGAIN) {
	  LOG(WARNING) << "connect failed: errno=" << errno << "\n";
	} else {
	  watch_read=false;
	  watch_write=true;
	  ret = 0;
	}
      } else {
	connected=true;
	signal();
      }
    }

    if (add) {
      addToFileList();
    }

    return ret;
  }

  virtual string read(int maxlen) { return FileHandle::read(maxlen); }

  NetworkAddress *getSocksServer();

  bool isConnected() {
    Synchronized(this);
    return connected;
  }

  int waitForConnect() {
    Synchronized(this);
    while ((!connected || connect_helper != NULL) && !hadError()) {
      wait();
    }
    if (hadError()) {
      return 1;
    }
    return 0;
  }

  string waitForData() {
    Synchronized(this);
    while (data.length() == 0 && !hadError()) {
      wait();
    }
    string d = data;
    data = string("");
    return d;
  }

  virtual void read(SocketEvent *se);

  virtual void error(textus::event::Event *se);

  virtual void canWrite(textus::event::Event *se) { 
    Synchronized(this); 
    watch_write = false;
    watch_read = true;
    AUTODEREF(NetworkAddress *, na);
    na = getPeerAddress();
    if (!na) {
      error(se);
      return;
    }
    connected = true; // maybe not, but we at least have results and can wake people up.
  }
};

class SocketFactory: virtual public Base {
public:
  SocketFactory() {}
  virtual ~SocketFactory() {}
  virtual Socket *createSocket() = 0;
  virtual Socket *create(Socket *parent, int fd) {
    Socket *s = createSocket();
    s->setParentSocket(parent);
    s->open(fd);
    return s;
  }
};

class SocketEvent: public textus::event::Event {
private:
  Error *err;
  string s;
  bool canWrite;

protected:
  virtual void do_it() {
    Socket *socket = dynamic_cast<Socket *>(eventTarget());
    if (socket) {
      if (err) {
	socket->error(this);
      }
      if (s.length()) {
	socket->read(this);
      }
      if (canWrite) {
	socket->canWrite(this);
      }
    }
  }

public:
  SocketEvent(Socket *sck): Event(sck), err(NULL), s(""), canWrite(false) {
  }

  virtual ~SocketEvent() {
  }

  string data() const { return s; }
  void setData(string str) {
    s = str;
  }

  const Error *error() const { return err; }

  void setError(Error *e) {
    if (err) {
      err->deref();
    }
    err = e;
    if (e) {
      e->ref();
    }
  }

  bool write() const { return canWrite; }
  void setWrite(bool w) { canWrite = w; }
  
};


class SocketEventFactory: public textus::file::FileHandleEventFactory {
private:
  AutoWeakDeref<Socket> socket;

protected:
public:
  explicit SocketEventFactory(Socket *ls): socket(ls) { }

  virtual ~SocketEventFactory() { }

  virtual textus::event::Event *errorEvent(Error *e);
  virtual textus::event::Event *readEvent(string s);
  virtual textus::event::Event *writeEvent();

  Socket *sock() {
    Synchronized(this);
    return socket;
  }

};

}} // namespace

#endif // TEXTUS_NETWORK_SOCKET_H_
