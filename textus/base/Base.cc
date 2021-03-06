/* Base.cc -*- C++ -*-
 * Copyright (C) 2009-2014 Ross Biro
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

#include <iomanip>

#include "textus/base/Base.h"
#include "textus/base/Thread.h"

namespace textus { namespace base {
int Base::instance_count = 0;

bool __attribute__( (weak) ) testing = false;

Base::~Base()
{
  // the 1 is for statically allocated things that were
  // never reffed or dereffed.
  if (ref_count != DEREF_MAGIC && ref_count != 1) {
    LOG(ERROR) << "Base deleted with a non-trivial reference count (" << ref_count <<"):\n";
    LOG_CALL_STACK(ERROR);
    die();
  }

  if (sync_count != 0) {
    LOG(ERROR) << "Base deleted with non-zero sync_count(" << sync_count <<"):\n";
    LOG_CALL_STACK(ERROR);
    die();
  }
  instance_count--;
}


std::ostream& Base::doPrint(std::ostream &o) const
{
  if (this) {
    // All we know about a base is that it's a base and it has an address.
    o << toString();
  }
  return o;
}

std::string Base::toString() const {
  char *buff;
  size_t len = 12 + 2*sizeof(void *); // really only need 9+2*sizeof...
  buff = reinterpret_cast<char *>(alloca(len));
  snprintf (buff, len, "Base(0x%p)", this);

  return string(buff);
}

std::ostream& operator<< (std::ostream& o, const Base& base)
{
  return base.doPrint(o);
}

std::ostream& operator<< (std::ostream& o, const Base * const base)
{
  return base->doPrint(o);
}

#ifdef DEBUG_LOCKS
int Base::currentLockLevel() const {
  Thread *s = Thread::self();
  if (s) {
    return s->getLockLevel();
  } else {
    return Thread::staticGetLockLevel();
  }
}

void Base::pushLockLevel(int ll) const {
  Thread *s = Thread::self();
  if (s) {
    s->pushLockLevel(ll);
  } else {
    Thread::staticPushLockLevel(ll);
  }
}

void Base::popLockLevel() const {
  Thread *s = Thread::self();
  if (s) {
    s->popLockLevel();
  } else {
    Thread::staticPopLockLevel();
  }
}  
#endif

}} //namespace
