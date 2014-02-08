/* ServerTemplate.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
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
    setcoordinator(NULL);
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
    setcoordinator(NULL);
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
  explicit ServerMessageWatcher(ServerSocket<T, S, P> *);
  virtual ~ServerMessageWatcher();
  void signalReceived();
  void close();
};


template <class T, class S, class P> class ServerActor: public textus::event::Actor {
private:
  AutoWeakDeref<ServerSocket<T, S, P> > parent;

public:
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
	AUTODEREF(watcher_type *, w);
	w = watcher;
	w->ref();
	watcher = NULL;
	w->close();
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
    Synchronized(this);
    string s = m->marshall();
    S::sendData(s);
  }

  void onConnect() {
    Synchronized(this);
    getProcessor()->onConnect();
    watcher->close();
    watcher=NULL;
  }
};

template <class T, class S, class P> class TReaderServer: public P {
private:
public:
  explicit TReaderServer(ServerSocket<T, S, P> *s);
  virtual ~TReaderServer() {}

  void messageRead(class P::EventType *lre);
  void eof(class P::EventType *lre);
};

template <class T, class S, class P> TReaderServer<T, S, P>::TReaderServer(ServerSocket<T, S, P> *s): P(s) 
{
}

template <class T, class S, class P> void TReaderServer<T, S, P>::messageRead(class P::EventType *lre)
{
  Synchronized(this);
  ServerSocket<T, S, P> *s = dynamic_cast<ServerSocket<T, S, P> *>(S::handle());
  s->messageRead(lre);
}

template <class T, class S, class P> void TReaderServer<T, S, P>::eof(class P::EventType *lre)
{
  Synchronized(this);
  ServerSocket<T, S, P> *s = dynamic_cast<ServerSocket<T, S, P> *>(S::handle());
  s->eof(lre);
}

template <class T, class S, class P> void ServerActor<T, S, P>::act(Message *m) {
  if (parent) {
    parent->processMessage(m);
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
  Synchronized(this);
  if (parent && parent->isConnected()) {
    parent->onConnect();
    close();
  }
}

template <class T, class S, class P> void
ServerMessageWatcher<T, S, P>::close()
{
  if (parent) {
    parent->removeWatcher(this);
    parent = NULL;
  }
}



}}} // namespace

#endif //TEXTUS_NETWORK_SERVER_SERVER_TEMPLATE_H_
