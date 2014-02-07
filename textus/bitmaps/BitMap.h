/* BitMap.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Abstract class to encapsulate all the different
 * kinds of bitmaps, so they can be used interchangably for
 * some things.
 *
 */

#ifndef TEXTUS_BITMAPS_BITMAP_H_
#define TEXTUS_BITMAPS_BITMAP_H_

#include "textus/base/Base.h"

namespace textus { namespace bitmaps {

class BitMap: virtual public textus::base::Base {
public:
  BitMap() {}
  virtual ~BitMap() {}

  virtual void set(size_t s) = 0;
  virtual void clear(size_t s) = 0;
  virtual bool isSet(size_t s) = 0;
  virtual bool isClear(size_t s) = 0;
};

}} //namespace

#endif // TEXTUS_BITMAPS_BITMAP_H_
