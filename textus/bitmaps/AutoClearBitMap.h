/* AutoClearBitMap.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * This is a scrolling bitmap that also
 * sets anything that has been out too
 * long.  It doesn't guarentee that older
 * bits will be set, but it tries to limit
 * the amount of memory in use.
 *
 */

#ifndef TEXTUS_BITMAPS_AUTO_CLEAR_BITMAP_H_
#define TEXTUS_BITMAPS_AUTO_CLEAR_BITMAP_H_

#include "textus/bitmaps/ScrollingBitMap.h"

namespace textus { namespace bitmaps {

class AutoClearBitMap: public ScrollingBitMap {
  time_t last_realloc;
protected:
  virtual void realloc(size_t s, size_t n) {
    if ((n - s) >= 128) {
      s = n - 96;
    }
    ScrollingBitMap::realloc(s, n);
  }

public:
  AutoClearBitMap() {}
  virtual ~AutoClearBitMap() {}

  

};

}} //namespace

#endif // TEXTUS_BITMAPS_AUTO_CLEAR_BITMAP_H_
