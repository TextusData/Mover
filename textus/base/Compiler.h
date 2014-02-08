/* Compiler.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Compiler specific things like memory barriers.
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

#ifndef TEXTUS_BASE_COMPILER_H_
#define TEXTUS_BASE_COMPILER_H_

#define wmb() do { __asm__ __volatile__ ("" ::: "memory"); } while (0)
#define rmb() do { __asm__ __volatile__ ("" ::: "memory"); } while (0)
#define mb() do  { __asm__ __volatile__ ("" ::: "memory"); } while (0)

#endif //TEXTUS_BASE_COMPILER_H_
