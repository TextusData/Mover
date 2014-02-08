/* OneTimeUse.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Template for one time use thingies.  Needs a lot of integer like properties to work
 * properly.
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

#include "textus/base/OneTimeUse.h"
namespace textus { namespace base {

DEFINE_INT_ARG(one_time_chunk_default_reserve, 16, "one_time_chunk_default_reserve", "How big of chunks to reserve for each thread for one time use variables.");

}} //namespace
