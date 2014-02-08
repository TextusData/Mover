/* SysRand.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * The system random number generator.
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
#ifndef TEXTUS_RANDOM_SYSRAND_H_
#define TEXTUS_RANDOM_SYSRAND_H_

#include "textus/base/ReferenceList.h"
#include "textus/random/PRNG.h"
#include "textus/file/TextusFile.h"

namespace textus { namespace random {

class SysRand: public PRNG {
private:
  unsigned char buffer[4096];
  size_t buff_ptr;
  textus::base::ReferenceList<textus::file::TextusFile *> file_handles;

protected:
  virtual uint64_t randombits(int bits);
  virtual void reloadBuffer();
  virtual void openFiles();

public:
  explicit SysRand(): buff_ptr(-1) {}
  virtual ~SysRand() {}
};


class SysRandFactory: public PRNGFactory {
 public:
  explicit SysRandFactory() {}
  virtual ~SysRandFactory() {}
  virtual PRNG* create(const std::string &name) { return new SysRand(); }

};


}} //namespace

#endif //TEXTUS_RANDOM_SYSRAND_H_
