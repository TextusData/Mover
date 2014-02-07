/* ListExtensions.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * A Mover Extension that lists the installed extensions.
 */

#ifndef TEXTUS_MOVER_LIST_EXTENSIONS_H_
#define TEXTUS_MOVER_LIST_EXTENSIONS_H_

#include "MoverExtension.h"

namespace textus { namespace mover {

class ListExtensionsRequest: public MoverExtension {
private:
public:
  ListExtensionsRequest() {}
  virtual ~ListExtensionsRequest() {}
  virtual void processIncoming(Mover *, MoverMessageProcessor *, std::string);
};

class ListExtensionsReply: public MoverExtension {
private:
public:
  ListExtensionsReply() {}
  virtual ~ListExtensionsReply() {}
  virtual void processIncoming(Mover *, MoverMessageProcessor *, std::string);
};

}} //namespace

#endif // TEXTUS_MOVER_LIST_EXTENSIONS_H_
