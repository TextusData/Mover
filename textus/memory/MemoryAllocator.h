/* MemoryAllocator.h -*- C++ -*-
 * Copyright (C) 2011 Ross Biro
 *
 * Implements a memory allocator.
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

#ifndef TEXTUS_MEMORY_MEMORYALLOCATOR_H_
#define TEXTUS_MEMORY_MEMORYALLOCATOR_H_

#include "textus/base/Base.h"
#include "textus/base/ThreadLocal.h"

namespace textus { namespace memory {
using namespace textus::base;

struct SmallBlock;
struct SmallBlockRoot;

class Heap: public textus::base::Base {
protected:
  size_t small_block_chunk_size;
  size_t small_block_max_size;
  size_t chunk_size;
  int64_t small_block_signature;
  SmallBlockRoot **small_block_trees;
  size_t small_block_count;

  static Heap *heapFromVPtr(void *);

  void *allocateSmall(size_t s);
  void *allocateLarge(size_t s);
  SmallBlock *smallBlockPtr(void *vptr);

  void freeSmall(SmallBlock *sb, void *vptr);
  void freeLarge(void *vptr);

  SmallBlockRoot *allocateSmallBlockRoot(int ind);
  void *locateOrAllocateSmallBlock(SmallBlockRoot *sbr, size_t count);

public:
  Heap();
  int Init();
  virtual ~Heap() {}
  void *allocate(size_t s);
  static void free(void *p);
};

class MemoryAllocator: public textus::base::Base {
private:
  ThreadLocal<Heap *> heap;
  
public:
  void *allocate(size_t s) {
    if (!heap) {
      heap = new Heap();
    }
    assert (heap != NULL);
    return heap->allocate(s);
  }

  void free(void *p) {
    Heap::free(p);
  }
    
};

void *allocate(size_t s);
void free(void *vptr);

}} //namespace

#endif // TEXTUS_MEMORY_MEMORYALLOCATOR_H_
