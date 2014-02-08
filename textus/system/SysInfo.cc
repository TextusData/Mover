/* SysInfo.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Handles all the OS specific stuff of getting number of cpus, cpu packages, etc.
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

#include "textus/system/SysInfo.h"

namespace textus { namespace system {

SysInfo *SysInfo::systemInformation() {
  static SysInfo si;
  return &si;
}

}} //namespace
