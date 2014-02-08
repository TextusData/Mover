/* ListeningSocket.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a network socket in listening mode.
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

#ifndef TEXTUS_NETWORK_LISTENING_SOCKET_H_
#define TEXTUS_NETWORK_LISTENING_SOCKET_H_

#include <string>

#include "textus/file/FileHandle.h"
#include "textus/base/Base.h"
#include "textus/file/FileHandleEventFactory.h"
#include "textus/event/Event.h"
#include "textus/base/logic/Bool.h"
#include "textus/network/Socket.h"
#include "textus/base/Error.h"
#include "textus/event/RoundRobinEventQueueScheduler.h"
#include "textus/event/EventQueue.h"

namespace textus { namespace network {

class ListeningSocket: public Socket {
private:
  textus::base::logic::Bool bound;
  textus::base::logic::Bool listening;
  AutoDeref<SocketFactory> childFactory;
  ReferenceList<Socket *> children;

protected:
  virtual int weakDelete() {
    ReferenceList<Socket *>::iterator it;
    ReferenceList<Socket *>::iterator cur;

    for (it = children.begin(); it != children.end(); ) {
      cur = it;
      ++it;
      (*cur)->setParentSocket(NULL);
      (*cur)->close();
    }
    children.clear();
    return Socket::weakDelete();
  }

  virtual void dataAvailable() {
    Synchronized(this);
    AUTODEREF(textus::event::Event *, ev);
    if (eventFactory()) {
      static string s(" ");
      ev = eventFactory()->readEvent(s);
      if (ev) {
	if (eventQueueScheduler()) {
	  ev->post(eventQueueScheduler()->getQueue());
	} else {
	  ev->post();
	}
      } else {
	LOG(ERROR) << "Unable to construct FileHandle error event.\n";
	die();
      }
    }
  }

public:
  ListeningSocket(NetworkAddress *na=NULL): bound(false), listening(false) {
    int fd;
    socklen_t slen;
    const sockaddr *sa;
    if (na) {
      sa = na->sockAddress(&slen);
    }
    {
      EXEC_BARRIER();
      fd = socket(na ? sa->sa_family:AF_INET, SOCK_STREAM, 0);
      open(fd);
    }
    if (na) {
      if (bind(na)) {
	return;
      }
      bound = true;
    }

    // Doesn't live long enough to do an auto release.
    SocketEventFactory *ef = new SocketEventFactory(this);
    if (ef) {
      setEventFactory(ef);
      ef->deref();
      ef = NULL;
    }

    if (listen()) {
      return;
    }
    bound = true;
    listening = true;
  }

  virtual ~ListeningSocket() {
    ReferenceList<Socket *>::iterator it;
    ReferenceList<Socket *>::iterator cur;
    close();
    for (it = children.begin(); it != children.end(); ) {
      cur = it;
      ++it;
      (*cur)->setParentSocket(NULL);
      (*cur)->close();
    }
    children.clear();
  }

  void error(Error *e) {
    LOG(ERROR) << e;
  }

  virtual void read(SocketEvent *se) {
    sockaddr addr;
    socklen_t addr_size = sizeof(addr);
    int fd;
    {
      EXEC_BARRIER();
      fd = ::accept(nativeHandle(), &addr, &addr_size);
      if (fd < 0) {
	AUTODEREF(Error *, e);
	e = new Error();
	error(e);
	return;
      }

      fcntl(fd, F_SETFD, FD_CLOEXEC);
    }

    if (childFactory) {
      AUTODEREF(Socket *, newsocket);
      newsocket = childFactory->create(this, fd);
      if (newsocket) {
	AUTODEREF(NetworkAddress *, peeraddr);
	peeraddr = new NetworkAddress(addr, addr_size);
	newsocket->setPeerAddress(peeraddr);
	AUTODEREF(textus::event::EventQueueScheduler *, eqs);
	eqs = new textus::event::EventQueueScheduler();
	newsocket->setEventQueueScheduler(eqs);
	newsocket->setEventQueue(textus::event::EventQueue::defaultQueue());
	children.push_back(newsocket);
      }
    } else {
      ::close(fd);
    }
    
  }

  virtual void childDead(Socket *child) {
    child->setParentSocket(NULL);
    children.erase(child);
  }

  SocketFactory *socketFactory() {
    Synchronized(this);
    return childFactory;
  }

  void setSocketFactory(SocketFactory *sf) {
    Synchronized(this);
    childFactory = sf;
  }

  void addEventQueue(textus::event::EventQueue *eq) {
    textus::event::RoundRobinEventQueueScheduler *rr =
      dynamic_cast<textus::event::RoundRobinEventQueueScheduler *>(eventQueueScheduler());
    rr->addQueue(eq);
  }

  void eraseEventQueue(textus::event::EventQueue *eq) {
    textus::event::RoundRobinEventQueueScheduler *rr =
      dynamic_cast<textus::event::RoundRobinEventQueueScheduler *>(eventQueueScheduler());
    rr->eraseQueue(eq);
  }

  void close() {
    ReferenceList<Socket *>::iterator it;
    ReferenceList<Socket *> c;
    {
      // can't lock this while calling close. 
      Synchronized(this);
      for (it = children.begin(); it != children.end(); ++it ) {
	c.push_back(*it);
      }
      children.clear();
    }
    for (it = c.begin();  it != c.end(); ++it) {
      Synchronized(*it);
      (*it)->setParentSocket(NULL);
      (*it)->close();
    }
    c.clear();
    Socket::close();
  }
};

}} //namespace

#endif // TEXTUS_NETWORK_LISTENING_SOCKET_H_
