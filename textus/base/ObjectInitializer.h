/* ObjectInitializer.h -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 * Contains the base classes that represents a type as stored in a data base.
 */

#ifndef TEXTUS_BASE_OBJECTINITIALIZER_H_
#define TEXTUS_BASE_OBJECTINITIALIZER_H_

#include "textus/base/Base.h"

namespace textus { namespace base {



class ObjectInitializer: virtual public Base {
private:
  static int initialize(const InitializerList *l) {
    for (InitializerList::iterator it = l->begin(); it != l->end(); ++it) {
      *it->doIt();
    }
    return 0;
  }
public:
  ObjectInitializer(const InitializerList *classInitializer, const InitializerList *objectInitializer) {
    static int doOnce = initialize(classInitializer);
    initialize(objectInitializer);
  }

  addInitializer(InitializerList *l, 
};

}} //namespace

#endif //TEXTUS_BASE_OBJECTINITIALIZER_H_
