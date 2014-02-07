/* SecureStorage.cc -*- C++ -*-
 * Copyright (C) 2011 Ross Biro
 *
 * Implements a "secure" new/delete system using mlock.
 *
 */

#include "textus/memory/SecureStorage.h"

namespace textus { namespace memory {

void SecureMemory::operator new(size_t size) {
}

void SecureMemory::operator delete(void *p) {
}

}} //namespace
