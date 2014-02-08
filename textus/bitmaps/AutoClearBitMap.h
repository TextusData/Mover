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
