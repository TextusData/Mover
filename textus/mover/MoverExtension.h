/* MoverExtension.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Encapsulate extensions to the mover class.
 */

#ifndef TEXTUS_MOVER_MOVER_EXTENSION_H_
#define TEXTUS_MOVER_MOVER_EXTENSION_H_

#include "textus/base/Base.h"
#include "textus/base/UUID.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/Mover.h"

namespace textus { namespace mover {

using namespace textus::base;
class Mover;
class MoverMessageProcessor;

class MoverExtension: virtual public Base {
private:
  static ReferenceValueMap<UUID, MoverExtension *> extensions;

public:
  static MoverExtension *lookup(UUID u) {
    Synchronized(&extensions);
    ReferenceValueMap<UUID, MoverExtension *>::iterator i = extensions.find(u);
    if (i != extensions.end()) {
      MoverExtension *mex = i->second;
      mex->ref();
      return mex;
    }
    return NULL;
  }

  static void registerExtension(UUID u, MoverExtension *mex) {
    Synchronized(&extensions);
    extensions[u] = mex;
  }

  static set<UUID> registeredExtensions() {
    Synchronized(&extensions);
    set<UUID> s;
    for (ReferenceValueMap<UUID, MoverExtension *>::const_iterator i = extensions.begin();
	 i != extensions.end();
	 ++i) {
      s.insert(i->first);
    }
    return s;
  }

  static Base *extensionLock() {
    return &extensions;
  }

  MoverExtension() {}
  virtual ~MoverExtension() {}
  virtual void processIncoming(Mover *, MoverMessageProcessor *, std::string) = 0;
};

}} //namespace


#endif // TEXTUS_MOVER_MOVER_EXTENSION_H_
