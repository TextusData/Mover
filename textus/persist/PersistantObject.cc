/* PersistantObject.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A class to represent a persistant object.  Persistant objects are
 * non-cache coherent, so effort must be taken to verify any updates
 * are handled properly.  Also there is always the potential for net
 * splits, so merges are inevitable as each side of the split updates
 * the object.
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

#include "textus/persist/PersistantObject.h"
#include "textus/base/init/Init.h"

namespace textus { namespace persist {

using namespace std;

PersistantObject::store_map PersistantObject::objectStores;

void PersistantObject::registerObjectStore(string type, PersistantObjectStore *pos) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&objectStores);
  objectStores[type] = pos;
}

void PersistantObject::eraseObjectStore(string type) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&objectStores);
  objectStores.erase(type);
}

PersistantObject *PersistantObject::fetch(string type, string locator) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&objectStores);
  smIterator i = objectStores.find(type);
  if (i != objectStores.end()) {
    PersistantObjectStore *pos = i->second;
    return pos->loadObject(type, locator);
  }
  return NULL;
}

}} //namespace
