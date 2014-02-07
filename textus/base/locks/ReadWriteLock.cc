/* ReadWriteLock.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 * Actually implements the lock.
 *
 */

#include "textus/base/locks/ReadWriteLock.h"

namespace textus { namespace base { namespace locks {

int ReadWriteLock::repairWriteLock() {
  int l = __builtin_popcountll(count & write_locked);
  int u = __builtin_popcountll(count & write_unlocked);
  LOG(ERROR) << "ReadWriteLock Corrupt write lock:\n";
  if (l > u) {
    count = (count & ~(write_locked | write_unlocked)) | write_locked;
    return 1;
  } else if (u > l) {
    count = (count & ~(write_locked | write_unlocked)) | write_unlocked;
    return 0;
  } else {
    LOG(ERROR) << "ReadWriteLock unrepairable write lock:\n";
    die();
  }
  /* Not Reached */
  assert(0);
  return -1;
}

int ReadWriteLock::repairReadLock() {
  LOG(ERROR) << "ReadWriteLock Corrupt read lock:\n";
  int v1 = count & 0xFFFF;
  int v2 = (count >> 16) & 0xFFFF;
  int v3 = (count >> 32) & 0xFFFF;

  if (v1 == v2) {
    count = (count & ~0XFFFF00000000) | ((long long)v1 << 32);
  } else if (v1 == v3) {
    count = (count & ~0XFFFF0000) | ((long long)v1 << 16);
  } else if (v2 == v3) {
    count = (count & ~0XFFFF) | v2;
  } else {
    LOG(ERROR) << "ReadWriteLock uncorrectable read lock.\n";
    die();
  }

  return count & 0xFFFF;
}

}}} // namespace textus::base::locks
