/* ProtoBufObject.cc -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A class that represents a persistant object stored
 * in the file system in the form of a protocol buffer.
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
  if (pbr && pbr->handle()) {
    pbr->handle()->setContext(NULL);
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
