/* ScrollingBitmap.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * represents a bunch of booleans.  This one assumes everything with
 * a smaller index than it knows is set, and everything with a
 * bigger index than it knows is clear.  It uses this to 
 * keep the bitmaps small.
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

#ifndef TEXTUS_BITMAPS_SCROLLING_BITMAP_H_
#define TEXTUS_BITMAPS_SCROLLING_BITMAP_H_

#include "textus/bitmaps/BitMap.h"

namespace textus { namespace bitmaps {

class ScrollingBitMap: public BitMap {
private:
  unsigned long *bits;

  size_t end;
  size_t start;

  size_t end_word;
  size_t alloced_words;
  size_t realloc_threshold;

  unsigned long *buffer;

protected:
  virtual void realloc(size_t s, size_t n) {
    assert (s % sizeof(*bits) == 0);
    assert (n % sizeof(*bits) == 0);
 
    if (s == start && n == end) {
      LOG(INFO) << "ScrollingBitMap::realloc called for no change.\n";
      return;
    }

    if (s < start) {
      // adding some to the beginning.
      size_t needed_words = (start - s) * sizeof(*bits);
      if (((unsigned long)bits - (unsigned long)buffer)/sizeof(*bits) > needed_words) {
	bits -= needed_words;
	start = s;
      } else {
	// need to move everything, or realloc.
	if (alloced_words - end_word <  needed_words) {
	  size_t new_count = alloced_words - end_word + needed_words + realloc_threshold;
	  size_t bits_offset = (unsigned long)bits - (unsigned long)buffer;
	  buffer = reinterpret_cast<unsigned long *>(::realloc(buffer, sizeof(*buffer) * new_count));
	  alloced_words = new_count;
	  bits = buffer + (bits_offset / sizeof (*buffer));
	}
	if (end_word) {
	  memmove(bits, bits + needed_words, end_word * sizeof(*bits));
	}

	memset(bits, -1, needed_words * sizeof(bits));
	end_word += needed_words;
	start = s;
      }
    } else if (s > start) {
      // removing something from the begginning.
      // just move the pointer forward.
      size_t freed_words = s-start;
      end_word -= freed_words;
      bits += freed_words;
      start = s;
    }

    if (n < end) {
      end_word = (n - start)/sizeof(*bits);
    } else if (n > end) {
      size_t needed_words = (n - start)/sizeof(*bits);
      if (needed_words > alloced_words) {
	size_t new_count = needed_words - alloced_words + realloc_threshold;
	size_t bits_offset = (unsigned long)bits - (unsigned long)buffer;
	buffer = reinterpret_cast<unsigned long *>(::realloc(buffer, sizeof(*buffer) * new_count));
	alloced_words = new_count;
	bits = buffer + (bits_offset / sizeof(*buffer));
	memset (buffer + end_word, 0, (alloced_words - end_word) * sizeof(*buffer));
      }
      end_word = needed_words;
      end = n;
    }
  }
  
public:
  ScrollingBitMap(): bits(NULL), end(0), start(0), end_word(0), alloced_words(0), realloc_threshold(16), buffer(NULL) {}
  virtual ~ScrollingBitMap() {
    if (buffer) {
      free(buffer);
    }
  }

  virtual bool isClear(size_t s) {
    return !isSet(s);
  }
  
  virtual bool isSet(size_t s) {
    if (s >= end) {
      return false;
    }
    if (s < start) {
      return true;
    }

    s = s - start;

    size_t bit = s & (sizeof(*bits) * 8 - 1);
    size_t word = s >> (3 * sizeof(*bits));

    return (bits[word] & (1 << bit));
  }

  virtual void set(size_t s) {
    if (s < start) {
      size_t new_start = (s & (sizeof(*bits) * 8 - 1)) * sizeof(*bits);
      realloc(new_start, end);
    }

    if (s >= end) {
      size_t new_end = ((s & (sizeof(*bits) * 8 - 1)) + 1) * sizeof(*bits);
      realloc(start, new_end);
    }

    // can happen if realloc fails, or
    // decides to do something different.
    if (s < start || s >= end) {
      return;
    }

    s = s - start;

    size_t bit = s & (sizeof(*bits) * 8 - 1);
    size_t word = s >> (3 * sizeof(*bits));
    bits[word] |= (1 << bit);
    size_t count = 0;
    while (word == 0 && bits[count] == (unsigned long)-1) {
      ++count;
    }

    if (count) {
      realloc(start + count*sizeof(*bits), end);
    }

  }

  virtual void clear(size_t s) {
    if (s < start) {
      size_t new_start = (s & (sizeof(*bits) * 8 - 1)) * sizeof(*bits);
      realloc(new_start, end);
    }

    if (s >= end) {
      size_t new_end = ((s & (sizeof(*bits) * 8 - 1)) + 1) * sizeof(*bits);
      realloc(start, new_end);
    }

    // can happen if realloc fails, or
    // decides to do something different.
    if (s < start || s >= end) {
      return;
    }

    s = s - start;

    size_t bit = s & (sizeof(*bits) * 8 - 1);
    size_t word = s >> (3 * sizeof(*bits));
    bits[word] &= ~(1 << bit);
    size_t count = 0;
    while (word == end_word && bits[word-count] == 0) {
      ++count;
    }
    if (count) {
      realloc(start, end - count*sizeof(*bits));
    }
  }
};

}} //namespace


#endif // TEXTUS_BITMAPS_SCROLLING_BITMAP_H_

 
