/* SecureStorage.cc -*- C++ -*-
 * Copyright (C) 2011 Ross Biro
 *
 * Implements a "secure" new/delete system using mlock.
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

#include "textus/memory/SecureStorage.h"

namespace textus { namespace memory {

void SecureMemory::operator new(size_t size) {
}

void SecureMemory::operator delete(void *p) {
}

}} //namespace
