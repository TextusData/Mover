/* Compiler.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Compiler specific things like memory barriers.
 *
 */

#ifndef TEXTUS_BASE_COMPILER_H_
#define TEXTUS_BASE_COMPILER_H_

#define wmb() do { __asm__ __volatile__ ("" ::: "memory"); } while (0)
#define rmb() do { __asm__ __volatile__ ("" ::: "memory"); } while (0)
#define mb() do  { __asm__ __volatile__ ("" ::: "memory"); } while (0)

#endif //TEXTUS_BASE_COMPILER_H_
