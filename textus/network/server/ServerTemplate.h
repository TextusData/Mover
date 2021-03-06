/* ServerTemplate.h -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * Class for a simple server.
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

#ifndef TEXTUS_NETWORK_SERVER_SERVER_TEMPLATE_H_
#define TEXTUS_NETWORK_SERVER_SERVER_TEMPLATE_H_

#include "textus/base/Base.h"
#include "textus/event/Message.h"
#include "textus/network/Socket.h"

namespace textus { namespace network { namespace server {

class ServerCoordinator: virtual public Base {
private:
protected:
  virtual int weakDelete() {
    return Base::weakDelete();
  }

public:
};

class Processor: virtual public textus::event::EventTarget {
private:
  MVAR(public, ServerCoordinator, coordinator);

protected:
  virtual int weakDelete() {
    set_coordinator(NULL);
    return textus::event::EventTarget::weakDelete();
  }

public:
  explicit Processor() {}
  virtual ~Processor() { close(); }
  virtual void processMessage(textus::event::Message *m) {
    LOG(ERROR) << "Base Processor processMessage called.\n";
    die();
  }

  virtual void processMessage(std::string m) {
    LOG(ERROR) << "Base Processor processMessage called.\n";
    die();
  }

  virtual void close() {
    set_coordinator(NULL);
  }

  virtual void onConnect() { }
};

// T = processor
// S = type of socket
// P = reader, i.e. line reader, message reader, etc.
template <class T, class S, class P> class TReaderServer;
template <class T, class S, class P> class ServerSocket;

using namespace textus::file;

template <class T, class S, class P>
class ServerMessageWatcher: public textus::event::EventWatcher {
private:
  AutoDeref<ServerSocket<T, S, P> > parent;

public:
  typedef ServerSocket<T, S, P> ServerSocketType;
  explicit ServerMessageWatcher(ServerSocket<T, S, P> *);
  virtual ~ServerMessageWatcher();
  void signalReceived();
  void close();
};


template <class T, class S, class P> class ServerActor: public textus::event::Actor {
private:
  AutoWeakDeref<ServerSocket<T, S, P> > parent;

public:
  typedef ServerSocket<T, S, P> ServerSocketType;
  explicit ServerActor(ServerSocket<T, S, P> *p):parent(p) {}
  virtual ~ServerActor() {}
  virtual void act(Message *m);
  virtual void close() {
    Synchronized(this);
    parent = NULL;
  }
};

template <class T, class S, class P> class ServerSocket: public S {
private:
  typedef ServerMessageWatcher<T, S, P> watcher_type;
protected:
  AutoDeref<P> lr; // this causes a circular reference. Neither the
		   // ProtoBufReader or the socket will go away until
		   // something happens
  typedef ServerActor<T, S, P> Actor;

  AutoDeref<T> processor;

  AutoDeref<watcher_type > watcher;

  virtual int weakDelete() {
    Synchronized(this);
    lr = NULL;
    if (processor) {
      AUTODEREF(T *, p);
      p = processor;
      p->ref();
      processor = NULL;
      p->close();
    }
    if (S::context()) {
      S::context()->close();
      S::setContext(NULL);
    }
    if (watcher != NULL) {
      watcher->close();
      watcher = NULL;
    }
    return S::weakDelete();
  }
  
public:
  ServerSocket() {

    AUTODEREF(Actor *, a);
    a = new Actor(this);
    S::setContext(a);

    AUTODEREF(watcher_type *, w);
    w = new watcher_type(this);
    watcher = w;

  }

  virtual ~ServerSocket() {}

  virtual void close() {
    {
      AUTODEREF(T *, tmpp);
      AUTODEREF(P *, tmpl);
      AUTODEREF(watcher_type *, tmpw);
    
      {
	Synchronized(this);
	if (lr) {
	  tmpl = lr;
	  tmpl->ref();
	  lr = NULL;
	}
	if (processor) {
	  tmpp = processor;
	  tmpp->ref();
	  processor = NULL;
	}
	/* XXXXXX FXIME, should this be here and not in S? */
	if (S::context()) {
	  S::context()->close();
	  S::setContext(NULL);
	}
	if (watcher != NULL) {
	  tmpw = watcher;
	  tmpw->ref();
	  watcher = NULL;
	}
      }
      if (tmpp) {
	tmpp->close();
      }
      if (tmpl) {
	tmpl->close();
      }

      if (tmpw) {
	tmpw->close();
      }
    }
    S::close();
  }

  virtual void eof(typename P::EventType *lre) {
    Synchronized(this);
    close();
  }

  void processMessage(std::string m) {
    Synchronized(this);
    processor->processMessage(m);
  }

  void processMessage(textus::event::Message *m) {
    Synchronized(this);
    processor->processMessage(m);
  }

  T *getProcessor() {
    Synchronized(this);
    return processor;
  }

  void sendMessage(std::string s) {
    Synchronized(this);
    S::sendData(s);
  }

  void sendMessage(textus::event::Message *m) {
    string s = m->marshall();
    S::sendData(s);
  }

  void onConnect() {
    AUTODEREF(T *, p);
    AUTODEREF(watcher_type *, w);
    {
      Synchronized(this);
      p = getProcessor();
      if (p) {
	p->ref();
      }
      
      w = watcher;
      if (w) {
	w->ref();
      }
      watcher = NULL;
    }

    if (p) {
      p->onConnect();
    }

    if (w) {
      w->close();
    }

  }
};

template <class T, class S, class P> class TReaderServer: public P {
private:
public:
  typedef ServerSocket<T, S, P> ServerSocketType;
  explicit TReaderServer(ServerSocketType *s);
  virtual ~TReaderServer() {}

  void messageRead(class P::EventType *lre);
  void eof(class P::EventType *lre);
};

template <class T, class S, class P> TReaderServer<T, S, P>::TReaderServer(ServerSocket<T, S, P> *s): P(s) 
{
}

template <class T, class S, class P> void TReaderServer<T, S, P>::messageRead(class P::EventType *lre)
{
  AUTODEREF(ServerSocketType *, s);
  {
    Synchronized(this);
    s = dynamic_cast<ServerSocketType *>(S::handle());
    if (s) {
      s->ref();
    }
  }
  if (s) {
    s->messageRead(lre);
  }
}

template <class T, class S, class P> void TReaderServer<T, S, P>::eof(class P::EventType *lre)
{
  AUTODEREF(ServerSocketType *, s);
  {
    Synchronized(this);
    s = dynamic_cast<ServerSocketType *>(S::handle());
    if (s) {
      s->ref();
    }
  }
  if (s) {
    s->eof(lre);
  }
}

template <class T, class S, class P> void ServerActor<T, S, P>::act(Message *m) 
{
  AUTODEREF(ServerSocketType *, p);
  {
    Synchronized(this);
    p = parent;
    if (p) {
      p->ref();
    }
  }
  if (p) {
    p->processMessage(m);
  }
}

template <class T, class S, class P>
ServerMessageWatcher<T, S, P>::ServerMessageWatcher(ServerSocket<T, S, P> *p):
  parent(p)
{
  parent->addWatcher(this);
  return;
}

template <class T, class S, class P> ServerMessageWatcher<T, S, P>::~ServerMessageWatcher()
{
  if (parent) {
    parent->removeWatcher(this);
  }
}

template <class T, class S, class P> void
ServerMessageWatcher<T, S, P>::signalReceived()
{
  AUTODEREF(ServerSocketType *, p);
  {
    Synchronized(this);
    p = parent;
    if (p) {
      p->ref();
    }
    close();
  }
  if (p && p->isConnected()) {
    p->onConnect();
  }
}

template <class T, class S, class P> void
ServerMessageWatcher<T, S, P>::close()
{
  AUTODEREF(ServerSocketType *, p);
  {
    Synchronized(this);
    p = parent;
    if (p) {
      p->ref();
      parent = NULL;
    }
  }
  if (p) {
    p->removeWatcher(this);
  }
}



}}} // namespace

#endif //TEXTUS_NETWORK_SERVER_SERVER_TEMPLATE_H_
