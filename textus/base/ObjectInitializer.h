/* ObjectInitializer.h -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 * Contains the base classes that represents a type as stored in a data base.
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
