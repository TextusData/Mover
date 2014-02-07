/* FileBackedNumber.h -*- c++ -*-
 * Copyright (c) 2013
 * Ross Biro
 *
 */
#ifndef TEXTUS_FILE_FILE_BACKED_NUMBER_H_
#define TEXTUS_FILE_FILE_BACKED_NUMBER_H_

#include <string>
#include <fcntl.h>

#include "textus/base/Base.h"
#include "textus/base/logic/Bool.h"
#include "textus/file/TextusFile.h"
#include "textus/base/OneTimeUse.h"

namespace textus { namespace file {

class FileBackedNumber: virtual public textus::base::Base {
private:
  textus::base::logic::Bool lock;
  AutoDeref<TextusFile> fh;

protected:
  void lockFile() { fh->lock(); }
  void unlockFile() { fh->unlock(); }
  TextusFile *handle() { return fh; }
  
public:
  explicit FileBackedNumber(): lock(false) {}
  virtual ~FileBackedNumber() {}

  int attachFile(std::string name) {
    int ret = 0;
    TextusFile *tf = TextusFile::openFile(name, O_RDWR|O_CREAT);
    HRNULL(tf);
    return  attachFile(tf);
  error_out:
    return ret;
  }

  int attachFile(TextusFile *t) {
    fh = t;
    return 0;
  }
  
  void setLock(textus::base::logic::Bool l=true) {
    Synchronized(this);
    lock = l;
  }

  uint64_t get();
  void set(uint64_t n);
  uint64_t increment(uint64_t n);
};

}} //namespace textus::file

namespace textus { namespace base {

template <> inline void
OneTimeUse<textus::file::FileBackedNumber,
	   uint64_t>::addChunk(ThreadLocal<uint64_t> *min,
			       ThreadLocal<uint64_t> *max) {
  *min = v.increment(reserve);
  *max = *min + reserve;
}

}} //namespace textus::base

#endif //TEXTUS_FILE_FILE_BACKED_NUMBER_H_
