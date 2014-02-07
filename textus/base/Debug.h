/* Debug.h -*- C++ -*-
 * Copyright (C) 2012 Ross Biro
 * 
 * Defines to enable debugging
 */

#ifndef TEXTUS_BASE_DEBUG_H_
#define TEXTUS_BASE_DEBUG_H_

#undef DEBUG_LOCKS
#define DEBUG_COND_BRK(x) do { if (x) { __asm__ __volatile__ ("int $0x03"); } } while (0)
#define DEBUG_BRK DEBUG_COND_BRK(1)

#endif // TEXTUS_BASE_DEBUG_H_
