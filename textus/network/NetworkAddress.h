/* NetworkAddress.h -*- c++ -*-
 * Copyright (c) 2010-2013 Ross Biro
 *
 * Represents a network address.
 *
 */

#ifndef TEXTUS_NETWORK_NETWORK_ADDRESS_H_
#define TEXTUS_NETWORK_NETWORK_ADDRESS_H_
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "textus/base/Base.h"
#include "textus/event/EventWatcher.h"

#include <string>
#include <list>
#include <map>

namespace textus { namespace network {
using namespace std;

class NetworkAddress: public textus::event::EventTarget {
private:
  struct address {
    socklen_t saddr_len;
    struct sockaddr *saddr;
  };

  typedef list<struct address> addresses;
  map<sa_family_t, addresses> assoc;
  std::string host;
  base::Time expires;
  bool valid;
  int err;

protected:
  void startResolve();

public:

  static NetworkAddress *resolve(std::string hostname);
  static NetworkAddress *addressFromString(std::string s);

  NetworkAddress(): valid(false), err(-1) { }
  NetworkAddress(const struct sockaddr &sa, socklen_t sa_len): valid(true), err(-1) {
    struct address addr;
    struct sockaddr *s2 =
      static_cast<struct sockaddr *>(malloc(sizeof(struct sockaddr)));
    memcpy (s2, &sa, sizeof(struct sockaddr));
    addr.saddr = s2;
    addr.saddr_len = sa_len;
    assoc[sa.sa_family].push_front(addr);
  }

  NetworkAddress(sa_family_t family, in_addr_t in_addr, in_port_t in_port): err(-1) {
    struct address addr;
    memset(&addr, 0, sizeof(addr));
    addr.saddr =
      static_cast<struct sockaddr *>(malloc(sizeof(struct sockaddr)));
    if (family == AF_INET) {
      addr.saddr_len = INET_ADDRSTRLEN;
      struct sockaddr_in *sin =
	reinterpret_cast<struct sockaddr_in *>(addr.saddr);
#ifdef HAVE_SIN_LEN
      sin->sin_len = addr.saddr_len;
#endif
      sin->sin_family = family;
      sin->sin_port = in_port;
      sin->sin_addr.s_addr = in_addr;
      assoc[family].push_front(addr);
      valid = true;
    } else {
      assert("NetworkAddress only deals with IN4 currently" == NULL);
    }
  }

  virtual ~NetworkAddress() {
    for (map<sa_family_t, addresses>::iterator i = assoc.begin();
	 i != assoc.end();
	 ++i) {
      for (addresses::iterator j = i->second.begin(); 
	   j != i->second.end();
	   ++j) {
	free(j->saddr);
      }
      i->second.clear();
    }
    assoc.clear();
  }

  virtual string toString() {
    if (!valid) {
      return string("invalid network address");
    }
    string o;
    for (map<sa_family_t, addresses>::iterator i = assoc.begin();
	 i != assoc.end(); ++i) {
      for (addresses::iterator j = i->second.begin();
	   j != i->second.end(); ++j ) {
	if (o.length() > 0) {
	  o = o + ", ";
	} else {
	  o = "NetworkAddress: <";
	}
	char buff[1024];
	if (inet_ntop(i->first, j->saddr, buff, sizeof(buff)) == NULL) {
	  LOG(ERROR) << "Unable to convert Network Address to String: "
		     << errno << "." << std::endl;
	  continue;
	}
	o = o + buff;
      }
    }
    o = o + ">";
    return o;
  }
  
  void setError(int e) {
    Synchronized(this);
    err = e;
    setValid(true);
  }

  int error() {
    Synchronized(this);
    return err;
  }

  bool isValid() {
    Synchronized(this);
    return valid;
  }

  void setValid(bool f) {
    Synchronized(this);
    valid = f;
    signal();
  }

  void waitForNameResolution() {
    Synchronized(this);
    while (!valid) {
      wait();
    }
  }

  bool operator == (NetworkAddress &n2) {
    Synchronized(this);
    Synchronized(&n2);
    if (this == &n2) {
      return true;
    }
    for (map<sa_family_t, addresses>::iterator i = assoc.begin(); i != assoc.end(); ++i) {
      map<sa_family_t, addresses>::iterator j = n2.assoc.find(i->first);
      if (j == n2.assoc.end()) {
	return false;
      }
      if (i->second.size() != j->second.size()) {
	return false;
      }
      for (addresses::iterator k = i->second.begin(); k != i->second.end(); ++k) {
	addresses::iterator l;
	for (l = j->second.begin(); l != j->second.end(); ++l) {
	  if ((*k).saddr_len != (*l).saddr_len)
	    continue;
	  if (memcmp((*k).saddr, (*l).saddr, 
		     min(static_cast<unsigned long>((*k).saddr_len), sizeof(struct sockaddr))) == 0) {
	      break;
	    }
	}
	if (l == j->second.end()) {
	  return false;
	}
      }
    }
    return true;
  }

  bool operator != (NetworkAddress &n2) {
    return ! (*this == n2);
  }

  std::string hostname() {
    Synchronized(this);
    return host;
  }

  void setHostname(std::string h) {
    Synchronized(this);
    host = h;
  }

  base::Time expiresTime() {
    Synchronized(this);
    return expires;
  }

  bool empty() {
    Synchronized(this);
    return assoc.empty();
  }

  void setExpiresTime(base::Time t) {
    Synchronized(this);
    expires = t;
  }

  void clearAddresses() {
    Synchronized(this);
    assoc.clear();
  }

  bool expired() {
    Synchronized(this);
    return valid && base::Time::now() >= expires;
  }

  void addAddress4(string d) {
    struct address addr;
    addr.saddr_len = INET_ADDRSTRLEN;
    addr.saddr =
      static_cast<struct sockaddr *>(malloc(sizeof(struct sockaddr)));
    struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(addr.saddr);
#ifdef HAVE_SIN_LEN
    sin->sin_len = addr.saddr_len;
#endif
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = *reinterpret_cast<const in_addr_t *>(d.c_str());
    assoc[sin->sin_family].push_back(addr);
  }

  void setPort(in_port_t port) {
    Synchronized(this);
    for (addresses::iterator i = assoc[AF_INET].begin(); i != assoc[AF_INET].end(); ++i) {
      struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(i->saddr);
      sin->sin_port = port;
    }
  }

  const struct sockaddr *sockAddress(socklen_t *sal) { 
    Synchronized(this);
    addresses l;
    l = assoc[AF_INET];
    if (l.empty()) {
      return NULL;
    }

    l.push_back(l.front());
    l.pop_front();
    *sal = l.back().saddr_len;
    return l.back().saddr; 
  }


};
}} //namespace

#endif // TEXTUS_NETWORK_NETWORK_ADDRESS_H_
