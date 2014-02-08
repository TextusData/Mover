/* Directory.cc -*- c++ -*-
 * Copyright (C) 2010-2013 Ross Biro
 *
 * A class to process directories and their entries
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

#include "textus/file/Directory.h"
#include "textus/file/SystemDirectory.h"
#include "textus/base/ReferenceValueMap.h"

#include <string.h>

namespace textus { namespace file {

using namespace std;

ReferenceValueMap<string, DirectoryProvider *> Directory::providers;

Directory *Directory::lookup(string path) {
  // Case 1 the path doesn't start with //, then it's a local system path.
  if (path[0] != '/' || path[1] != '/') {
    return new SystemDirectory(path);
  }

  string::size_type st = path.find('/', 2);
  string p;
  if (st == string::npos) {
    p = path.substr(2);
  } else {
    p = path.substr(2, st - 1);
  }

  assert(!textus::base::init::shutdown_done);
  Synchronized(&providers);
  ReferenceValueMap<string, DirectoryProvider *>::iterator i = providers.find(p);
  if (i == providers.end()) {
    return NULL;
  }

  DirectoryProvider *dp = i->second;
  return dp->provide(p);
}

void Directory::registerProvider(string nspace, DirectoryProvider *p) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&providers);
  providers[nspace] = p;
}

void Directory::eraseProvider(string nspace) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&providers);
  providers.erase(nspace);
}

Directory::Entries *Directory::entries() {
  Entries *e;
  e = new Entries();
  if (e == NULL) {
    return NULL;
  }
  rewind();
  while(!eod()) {
    e->push_back(current());
    next();
  }
  return e;
}

bool Directory::recursiveUnlink() {
  while (!eod()) {
    DirectoryEntry *c = current();
    if (c->isDir()) {
      AUTODEREF(Directory *, d);
      d = c->openDir();
      if (!d->recursiveUnlink()) {
	return false;
      }
    }
    if (!c->unlink()) {
      return false;
    }
  }
  return true;
}

// Only returns the first one matching a name.
DirectoryEntry *Directory::findEntry(string name) {
  textus::base::predicate::Filters<Entries> f;
  NameMatches n(name);
  AUTODEREF(Entries *, e);
  e = f.filterMatching(entries(), &n);
  if (e->empty()) {
    return NULL;
  }
  return e->front();
}

size_t Directory::entryCount() {
  size_t count = 0;
  for (Entries::iterator i = entries()->begin(); i != entries()->end(); ++i) {
    if ((*i)->name() != string(".") && (*i)->name() != string("..")) {
      count ++;
    }
  }
  return count;
}


}} //namespaces

