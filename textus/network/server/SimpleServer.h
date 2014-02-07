/* SimpleServer.h -*- c++ -*-
 * Copyright (c) 2009-2010, 2013 Ross Biro
 *
 * Base class of most servers in the system.
 *
 */

#include "textus/base/Base.h"
#include "textus/network/ListeningSocket.h"
#include "textus/network/NetworkAddress.h"
#include "textus/network/Socket.h"
#include "textus/event/EventWatcher.h"
#include "textus/network/URL.h"

#ifndef TEXTUS_NETWORK_SERVER_SIMPLE_SERVER_H_
#define TEXTUS_NETWORK_SERVER_SIMPLE_SERVER_H_

namespace textus { namespace network { namespace server {
class SimpleServer: virtual public textus::event::EventWatcher {
private:
  AutoDeref<ListeningSocket> sock;
  AutoDeref<NetworkAddress> addr;
  AutoDeref<SocketFactory> factory;
  int port;
  bool waiting_for_address;
  bool bound;
    
public:
  SimpleServer():port(0), waiting_for_address(false), bound(false) {}
  virtual ~SimpleServer() {}

  NetworkAddress *address() {
    Synchronized(this);
    return addr;
  }

  NetworkAddress *getAddress(void) {
    Synchronized(this);
    return addr;
  }

  void setAddress(URL *u) {
    int ret = 0;
    Synchronized(this);
    close();
    waiting_for_address = true;
    if (addr == NULL) {
      port = u->port();
      addr = NetworkAddress::resolve(u->hostname());
      HRNULL(addr);
      addr->addWatcher(this);
      eventReceived(NULL);
      return;
    }
    signalReceived();
  error_out:;
    if (ret != 0) {
      close();
    }
  }

  void waitForBind() {
    Synchronized(this);
    while (!bound) {
      wait();
    }
  }

  void addressResolved() {
    NetworkAddress *na = addr;
    addr = NULL;
    close();
    setAddress(na); // lots of side effects.
  }

  virtual void signalReceived() {
    Synchronized(this);
    if (waiting_for_address && addr->isValid()) {
      waiting_for_address = false;
      addressResolved();
    }
  }

  // Lots of side effects.  Creates a whole new listening socket.
  void setAddress(NetworkAddress *na) {
    Synchronized(this);
    if (addr && na && *addr == *na) {
      // just in case addr != na
      addr = na;
      return;
    }
      
    sock = NULL;

    // makesure that ls get's reffed/dereffed the right number of times.
    if (port != 0) {
      na->setPort(htons(port));
    }
    sock = new ListeningSocket(na);
    bound = true;
    signal();
    if (sock != NULL) {
      sock->deref();
      
      sock->setSocketFactory(factory);

      addr = sock->getAddress();
      if (addr) {
	addr->deref();
      }
    }

  }

  ListeningSocket *listeningSocket() {
    Synchronized(this);
    return sock;
  }

  void setListeningSocket(ListeningSocket *s) {
    Synchronized(this);
    sock = s;
  }

  SocketFactory *socketFactory() {
    Synchronized(this);
    return factory;
  }

  void setSocketFactory(SocketFactory *f) {
    Synchronized(this);
    factory = f;
    if (sock) {
      sock->setSocketFactory(f);
    }
  }

  virtual void sendData(string data) {
    LOG(ERROR) << "SimpleServer::sendData called.\n";
    LOG_CALL_STACK(ERROR);
    die();
  }

  virtual void close() {
    Synchronized(this);
    if(sock) {
      sock->close();
    }
  }
};
}}} // namespace


#endif // TEXTUS_NETWORK_SERVER_SIMPLE_SERVER_H_