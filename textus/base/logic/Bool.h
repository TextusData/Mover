/* Bool.h  -*- c++ -*-
 * A boolean class with some error protection.
 *
 * Copyright (c) 2009 Ross Biro
 * Confidential Source Code 
 * May contain trade secrets.
 * Do Not Divuldge.
 */

#ifndef BASE_LOGIC_BOOL_H_
#define BASE_LOGIC_BOOL_H_

#include <stddef.h>
#include "textus/base/logic/Bool.pb.h"

namespace textus { namespace base { namespace logic {

//MonitoredValue(int, CorruptBools, 0);
//MonitoredValue(int, UncorrectableBools, 0);

class Bool {
private:
  // We are assuming assignments to this are atomic.
  unsigned char value;
  
  enum {
    BTrue  = 0x0F,
    BFalse = 0xF0,
  };
  
  bool correct();
  
public:

  Bool():value(BFalse) {}

  Bool(int val):value((val)?BTrue:BFalse) {}

  Bool(bool val):value((val)?BTrue:BFalse) {}

  Bool(const Bool &other):value(other.value) {}

  Bool(const BoolPB &pb) {
    if (!pb.has_bool_value()) {
      value = BFalse;
    } else {
      value = pb.bool_value();
    }
    if (value != BFalse && value != BTrue) {
      correct();
    }
  }

  const bool test_c() {
    if (value == BTrue) return true;
    if (value == BFalse) return false;
    return correct();
  }

#if 0
  // In theory this can be optimized to be faster than the above
  // However, it doesn't appear that it has, and the performance needs
  // to be tested and verified.

  const bool test_asm() {
    int ret;
    __asm__ ("movb %1, %%al\n\t"
	     "add $0x11 %%al\n\t"
	     "jcs 1f\n\t"
	     "jas 2f\n\t"
	     "jmp parity_error\n\t"
	     "1: mov $0, %%eax\n\t"
	     "jmp 3f\n\t"
	     "2: mov $1, %%eax\n\t"
	     "3:\n" : "=a" (ret) : "=m" (value) );
    return ret;
  parity_error:
    return correct();
  }
#endif

#ifdef BOOL_TESTED_X86_INLINE_ASM
  operator bool () { return test_asm(); }
#else
  operator bool () { return test_c(); }
#endif

  operator BoolPB() const {
    BoolPB pb;
    pb.set_bool_value(value);
    return pb;
  }

  bool clear() {
    value = BFalse;
    return false;
  }

  bool set() {
    value = BTrue;
    return true;
  }

  bool set(bool b) {
    value = (b)?BTrue:BFalse;
    return b;
  }

};

}}} //namespace base::logic

#endif // BASE_LOGIC_BOOL_H_
