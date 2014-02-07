/* SecureStorage.h -*- C++ -*-
 * Copyright (C) 2011 Ross Biro
 *
 * Implements a "secure" new/delete system using mlock.
 *
 */

#ifndef TEXTUS_MEMORY_SECURESTORAGE_H_
#define TEXTUS_MEMORY_SECURESTORAGE_H_

namespace textus { namespace memory {

class SecureMemory: virtual public Base {
public:
  static void *operator new(size_t size);
  static void operator delete(void *p);
};

}} // namespace

#endif // TEXTUS_MEMORY_SECURESTORAGE_H_
