/* Debug.h -*- C++ -*-
 * Copyright (C) 2012 Ross Biro
 * 
 * Defines to enable debugging
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

#ifndef TEXTUS_BASE_DEBUG_H_
#define TEXTUS_BASE_DEBUG_H_

#undef DEBUG_LOCKS
#define DEBUG_COND_BRK(x) do { if (x) { __asm__ __volatile__ ("int $0x03"); } } while (0)
#define DEBUG_BRK DEBUG_COND_BRK(1)

#endif // TEXTUS_BASE_DEBUG_H_
