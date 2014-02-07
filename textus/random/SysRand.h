/* SysRand.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * The system random number generator.
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
