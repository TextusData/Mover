/* MemoryAllocator.cc -*- C++ -*-
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

#include "textus/memory/MemoryAllocator.h"
#include "textus/base/init/Init.h"

#include <unistd.h>
#include <errno.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/param.h>


namespace textus {
namespace memory {

using namespace base;

DEFINE_INT_ARG(allocator_small_chunk_size, 1, "allocator_small_chunk_size", "Size in pages of chunks allocated for small blocks.");
DEFINE_UINT_ARG(allocator_small_block_max_size, 128, "allocate_small_block_max", "Size in bytes of the largest allocation to be done in a small block as opposed to the main allocator.");
DEFINE_INT_ARG(allocator_chunk_size, 1, "allocator_chunk_size", "Size in pages of chunks to allocate for blocks that are too small to get their own set of pages.");
DEFINE_INT64_ARG(allocator_small_block_signature, 0xFEEDBEEFBABEEA80, "allocator_small_block_signature", "Signature for identifying small block allocator pages.");

struct SmallBlockRoot {
  size_t size;
  unsigned flags;
  struct SmallBlock *sb; // points to the first small block in the chain.
};

struct SmallBlock {
  size_t depth;
  size_t used;
  SmallBlockRoot *root;
  int64_t signature;
  size_t total;
  unsigned long bit_map[];
};


enum {
  ALLOCATE_FLAGS_LOCKED = 01,
};

static void *allocatePages(size_t count, int flags = 0, int prot = PROT_READ|PROT_WRITE) {
  
  void *ptr = mmap(NULL, count * PAGE_SIZE, prot, MAP_ANON, -1, 0);
  if (ptr == NULL) {
    LOG(ERROR) << "Unable to allocate pages " << count << " in allocate pages (errno = " << errno << "\n";
    return NULL;
  }
  if (flags & ALLOCATE_FLAGS_LOCKED) {
    if (mlock(ptr, count * PAGE_SIZE) < 0) {
      LOG(ERROR) << "Unable to lock pages in allocate pages: errno = " << errno << "\n";
      munmap(ptr, count * PAGE_SIZE);
      return NULL;
    }
  }

  return ptr;
}

static SmallBlock *allocateSmallBlockChild(SmallBlock **bl, size_t count) {
  SmallBlock *sbp = static_cast<SmallBlock *>(allocatePages(allocator_small_chunk_size, (*bl)->root->flags ));
  if (sbp == NULL) {
    return NULL;
  }
  memset(sbp, 0, PAGE_SIZE * allocator_small_chunk_size);
  sbp->root = (*bl)->root;
  sbp->depth = (*bl)->depth - 1;
  sbp->used = 0;
  if (sbp->depth == 0) {
    sbp->total = static_cast<int>((PAGE_SIZE*allocator_small_chunk_size - sizeof(*sbp))/sbp->root->size);
  } else {
    sbp->total = (*bl)->total/count;
  }
  return sbp;
}

static void *locateSmallBlock(SmallBlock **bl, size_t count);
static void *locateSmallBlockFromRoot(SmallBlockRoot *br, size_t count) {
  return locateSmallBlock(&br->sb, count);
}

static void *locateSmallBlock(SmallBlock **bl, size_t count) {
  if ((*bl)->used == (*bl)->total) {
    return NULL;
  }

  for (unsigned i = 0; i < count; i++) {
    if ((*bl)->bit_map[i]) {
      int fb = __builtin_ffsl((*bl)->bit_map[i]);
      assert (fb != 0);
      --fb;
      void **aptr = reinterpret_cast<void **>(&(*bl)->bit_map[count]);
      void *ptr = aptr[fb + i*sizeof((*bl)->bit_map[0])*8];
      if (ptr != NULL) {
	ptr = locateSmallBlock(reinterpret_cast<SmallBlock **>(aptr+fb+i*sizeof((*bl)->bit_map[0])*8), count);
	if (ptr == NULL) {
	  (*bl)->bit_map[i] &= ~(1 << fb);
	  continue;
	}
	(*bl)->used++;
	return ptr;
      }

      if (!(*bl)->depth) {
	(*bl)->bit_map[i] &= ~(1 << fb);
	(*bl)->used++;
	return aptr+fb+i*sizeof((*bl)->bit_map[0])*8;
      } else {
	aptr[fb+i*sizeof((*bl)->bit_map[0])*8] = allocateSmallBlockChild(bl, count);
	return locateSmallBlock(reinterpret_cast<SmallBlock **>(aptr+fb+i*sizeof((*bl)->bit_map[0])*8), count);
      }
    }
  }
  return NULL;
} 

static void *allocateSmallBlockParent(SmallBlockRoot *br, size_t count) {
  SmallBlock *sbp = static_cast<SmallBlock *>(allocatePages(allocator_small_chunk_size, br->flags ));
  if (sbp == NULL) {
    return NULL;
  }
  memset(sbp, 0, PAGE_SIZE * allocator_small_chunk_size);
  sbp->depth = br->sb->depth + 1;
  sbp->used = br->sb->used;
  sbp->total = count * br->sb->total;
  sbp->bit_map[0] = 1;
  sbp->root = br;
  SmallBlock **aptr = reinterpret_cast<SmallBlock **>(&sbp->bit_map[count]);
  aptr[0] = br->sb;
  return sbp;
}


void *Heap::locateOrAllocateSmallBlock(SmallBlockRoot *sbr, size_t count) {
  void *ptr = locateSmallBlockFromRoot(sbr, count);
  if (ptr != NULL) {
    return ptr;
  }
  
  ptr = allocateSmallBlockParent(sbr, count);
  sbr->sb = static_cast<SmallBlock *>(ptr);

    return locateOrAllocateSmallBlock(sbr, count);
}

Heap::Heap() {
  small_block_chunk_size = allocator_small_chunk_size;
  small_block_max_size = allocator_small_block_max_size;
  chunk_size = allocator_chunk_size;
  small_block_signature = allocator_small_block_signature;
  small_block_trees = NULL;
  small_block_count = 0;
}

SmallBlockRoot *Heap::allocateSmallBlockRoot(int index) {
  SmallBlockRoot *sbr = static_cast<SmallBlockRoot *>(malloc(sizeof(*sbr)));
  if (sbr == NULL) {
    return NULL;
  }
  memset (sbr, 0, sizeof(*sbr));
  sbr->size = 1 << index;
  return sbr;
}

int Heap::Init() {
  unsigned i;
  unsigned mask=1;
  for (i = 0; mask < small_block_max_size; ++i) {
    mask = mask << 1;
  }
  small_block_count = i;
  small_block_trees = static_cast<SmallBlockRoot **>(malloc( sizeof(*small_block_trees) * small_block_count));
  if (small_block_trees == NULL) {
    return -1;
  }

  for (i = 0; i < small_block_count; i++) {
    small_block_trees[i] = allocateSmallBlockRoot(i);
    if (small_block_trees[i] == NULL) {
      return -1;
    }
  }

  return 0;
}

void *Heap::allocateSmall(size_t s) {
  for (unsigned i = 0; i < small_block_count; i++) {
    if (s < small_block_trees[i]->size) {
      return locateOrAllocateSmallBlock(small_block_trees[i], static_cast<int>((PAGE_SIZE*allocator_small_chunk_size - sizeof(SmallBlock))/sizeof(void *)));
    }
  }
  LOG(ERROR) << "allocatSmall(" << s << ") called, but s is too big.\n";
  die();
  return NULL;
}

void Heap::freeSmall(SmallBlock *bl, void *vptr) {
  if (bl->signature != allocator_small_block_signature) {
    LOG(ERROR) << "Bad pointer in freeSmall.\n";
    die();
  }
  // Are there macro's for this somewhere?
  unsigned long offset = reinterpret_cast<unsigned long>(vptr) - reinterpret_cast<unsigned long>(bl);
  offset = offset - sizeof(*bl);
  unsigned long idx = offset / bl->root->size;
  unsigned long i = idx/sizeof(bl->bit_map[0]);
  unsigned long b = idx % sizeof(bl->bit_map[0]);
  bl->bit_map[i] |= (1 << b);
}


void *Heap::allocate(size_t s) {
  if (s <= small_block_max_size) {
    return allocateSmall(s);
  } 
  return allocateLarge(s);
}

SmallBlock *Heap::smallBlockPtr(void *vptr) {
  SmallBlock *bl = reinterpret_cast<SmallBlock *>(reinterpret_cast<unsigned long>(vptr) 
						  & ~(allocator_small_chunk_size * PAGE_SIZE - 1));
  if (bl->signature == allocator_small_block_signature) {
    return bl;
  } else {
    return NULL;
  }
}

Heap *Heap::heapFromVPtr(void *vp) {
  return NULL;
}

void Heap::freeLarge(void *vptr) {
}

void *Heap::allocateLarge(size_t s) {
  return NULL;
}

void Heap::free(void *vptr) {
  Heap *h = heapFromVPtr(vptr);
  SmallBlock *sb = h->smallBlockPtr(vptr);
  if (sb) {
    h->freeSmall(sb, vptr);
  } else {
    h->freeLarge(vptr);
  }
}


}} //namespace
