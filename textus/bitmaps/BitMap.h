/* BitMap.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Abstract class to encapsulate all the different
 * kinds of bitmaps, so they can be used interchangably for
 * some things.
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
