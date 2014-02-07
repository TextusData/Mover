/* Bool.h  -*- c++ -*-
 * A boolean class with some error protection.
 *
 * Copyright (c) 2009 Ross Biro
 * Confidential Source Code 
 * May contain trade secrets.
 * Do Not Divuldge.
 */


#include "textus/base/logic/Bool.h"
#include "textus/logging/Log.h"

namespace textus { namespace base { namespace logic {

static int CorruptBools = 0;

bool Bool::correct()
{
  if (value != BFalse && value != BTrue) {

    LOG(WARNING) << "Corrupt Boolean value: " << value << "\n";
    LOG_CALL_STACK(WARNING);
    CorruptBools++;

    int f = __builtin_popcount(value & BFalse);
    int t = __builtin_popcount(value & BTrue);

    if (f > t) {
      value = BFalse;
    } else if (f != t) {
      value = BTrue;
    } else {
      LOG(ERROR) << "Uncorrectable boolean: Assuming False\n";
      die();
    }
  }
  return (value == BTrue);
}

}}} //namespace
