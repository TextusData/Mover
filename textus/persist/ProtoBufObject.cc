/* ProtoBufObject.cc -*- C++ -*-
 * Copyright (C) 2010, 2014 Ross Biro
 *
 * A class that represents a persistant object stored
 * in the file system in the form of a protocol buffer.
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

#include "textus/persist/ProtoBufObject.h"
#include "textus/file/ProtoBufReaderEventFactory.h"
#include "textus/file/ProtoBufReader.h"

namespace textus { namespace persist {

using namespace textus::event;
using namespace textus::base;
using namespace textus::file;

class ProtoBufObjectActor: public Actor {
private:
  AutoWeakDeref<ProtoBufObject> pbo;

public:
  ProtoBufObjectActor(ProtoBufObject *p): pbo(p) {}
  virtual ~ProtoBufObjectActor() {}
  
  virtual void act(Message *m) {
    if (pbo) {
      pbo->setMessage(m);
    }
  }
};

int ProtoBufObject::weakDelete() {
  AUTODEREF(ProtoBufReader *, p);
  {
    Synchronized(this);
    p = pbr;
    if (p) {
      p->ref();
    }
    pbr = NULL;
  }

  if (p != NULL) {
    p->close();
  }

  return PersistantObject::weakDelete();
}

ProtoBufObject::ProtoBufObject(Message *m, string locator, int mode) {
  mess = m;
  AUTODEREF(FileHandle *, fh);
  fh = FileHandle::openFile(locator.c_str(), O_RDWR, mode);
  pbr = new ProtoBufReader(fh);
  AUTODEREF(ProtoBufObjectActor *,pboa);
  pboa = new ProtoBufObjectActor(this);
  fh->setContext(pboa);
}

ProtoBufObject::ProtoBufObject() {}

ProtoBufObject::~ProtoBufObject() {}

PersistantObject *ProtoBufObjectStore::loadObject(string type, string locator) {
  AUTODEREF(FileHandle *, fh);
  Synchronized(this);
  fh = root->openFile(locator, O_RDWR);


  ProtoBufObject *pbo;
  pbo = new ProtoBufObject();
  if (pbo == NULL) {
    return NULL;
  }

  AUTODEREF(ProtoBufObjectActor *, pboa);
  pboa = new ProtoBufObjectActor(pbo);
  fh->setContext(pboa);

  AUTODEREF(ProtoBufReader *, pbr);
  pbr = new ProtoBufReader(fh);
  if (pbr == NULL) {
    return NULL;
  }

  pbo->setProtoBufReader(pbr);

  AUTODEREF(ProtoBufReaderEventFactory *, pbref);
  pbref = new ProtoBufReaderEventFactory(pbr);

  pbr->setEventFactory(pbref);

  // Make sure the message hasn't already been read.
  AUTODEREF(Message *, m);
  ProtoBufReaderStatus::status stat;
  m = pbr->readMessage(stat);
  if (stat == ProtoBufReaderStatus::ok) {
    pbo->setMessage(m);
  }

  return static_cast<PersistantObject *>(pbo);
}

}} //namespace
