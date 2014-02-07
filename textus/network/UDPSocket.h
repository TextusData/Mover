/* UDPSocket.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a network packet socket.
 *
 */

#ifndef TEXTUS_NETWORK_UDP_SOCKET_H_
#define TEXTUS_NETWORK_UDP_SOCKET_H_

#include "textus/network/Socket.h"
#include "textus/network/NetworkAddress.h"

namespace textus { namespace network {

using namespace textus::event;

class UDPSocketEvent: public SocketEvent {
private:
  AutoDeref<NetworkAddress> src;

public:
  UDPSocketEvent(Socket *s): SocketEvent(s) {}
  virtual ~UDPSocketEvent() {}
  
  NetworkAddress *srcAddress() {
    Synchronized(this);
    return src;
  }

  void setSrcAddress(NetworkAddress *addr) {
    Synchronized(this);
    src = addr;
  }

};

class UDPSocketEventFactory: public SocketEventFactory {
public:
  explicit UDPSocketEventFactory(Socket *s):SocketEventFactory(s) {}
  virtual textus::event::Event *readEvent(string s, NetworkAddress *na);
  
};

class UDPSocket: public Socket {

protected:

  string recv(int maxlen, NetworkAddress **na) {
    Synchronized(this);

    if (nativeHandle() == -1) {
      watch_read = false;
      setEof();
      setError();
      LOG(ERROR) << "Inavlid file handle in read.\n";
      die();
    }

    AUTOFREE(void *, buff);
    buff = malloc(maxlen);

    if (buff == NULL) {
      LOG(ERROR) << "Out of memory allocating read buffer.\n";
      die();
      return string("");
    }

    sockaddr addr;
    socklen_t addr_len=sizeof(addr);

    int red = ::recvfrom(nativeHandle(), buff, maxlen, 0, &addr, &addr_len);

    struct sockaddr_in *sin = reinterpret_cast<sockaddr_in *>(&addr);
    memset(&sin->sin_zero, 0, sizeof(sin->sin_zero));

    if (red == 0) {
      watch_read = false;
      eof_flag = true;
      return string("");
    }

    if (red < 0) {
      if (errno != EAGAIN) {
	watch_read = false;
	setEof();
	setError(errno);
	return string("");
      }
      return string("");
    }

    *na = new NetworkAddress(addr, addr_len);
    return string((const char *)buff, (string::size_type)red);
  }


  virtual void dataAvailable() {
    Synchronized(this);
    static const int buffer_size=1024;
    if (eventFactory()) {
      NetworkAddress *na;
      for (string s = recv(buffer_size, &na); s != ""; s=recv(buffer_size, &na)) {
	AUTODEREF(Event *, ev);
	UDPSocketEventFactory *f = dynamic_cast<UDPSocketEventFactory *>(eventFactory());
	if (f != NULL) {
	  ev = f->readEvent(s, na);
	} else {
	  ev = eventFactory()->readEvent(s);
	}
	na->deref();
	if (ev) {
	  if (ev->preferredQueue() || !eventQueueScheduler()) {
	    ev->post();
	  } else {
	    ev->post(eventQueueScheduler()->getQueue());
	  }
	} else {
	  LOG(ERROR) << "Unable to create event.\n";
	  die();
	}
      }
    }
  }

public:
  UDPSocket(int addr_family = AF_INET) {
    EXEC_BARRIER();
    open(socket(addr_family, SOCK_DGRAM, 0));
  }

};


inline textus::event::Event *UDPSocketEventFactory::readEvent(string s, NetworkAddress *na) {
  Synchronized(this);
  UDPSocketEvent *e = new UDPSocketEvent(sock());
  e->setSrcAddress(na);
  e->setData(s);
  setEventQueue(e);
  return e;
}

}} //namespace

#endif // TEXTUS_NETWORK_UDP_SOCKET_H_
