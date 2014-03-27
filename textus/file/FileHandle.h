/* FileHandle.h -*- c++ -*-
 * Copyright (c) 2009-2014 Ross Biro
 *
 * Represents a system file handle.
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

#ifndef TEXTUS_FILE_FILE_HANDLE_H_
#define TEXTUS_FILE_FILE_HANDLE_H_

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>

#include "textus/file/TextusFile.h"
#include "textus/base/WeakReferenceList.h"
#include "textus/base/logic/Bool.h"
#include "textus/file/FileHandleEventFactory.h"
#include "textus/event/EventQueueScheduler.h"
#include "textus/threads/WorkerThread.h"
#include "textus/event/Actor.h"
#include "textus/file/Handle.h"

namespace textus { namespace file {
using namespace textus::base::logic;

class FileHandleEventFactory;

#define EXEC_BARRIER() Synchronized(textus::file::FileHandle::execBarrierObject())
#define GRAB_EXEC_BARRIER() EXEC_BARRIER()

class FileHandle: public Handle {
private:
  int fh;
  int eof_count;
  Bool non_blocking;
  Bool close_on_exec;
  Bool sent_eof;
  int read_count;
  Bool watch_zero_reads;
  Bool on_list;
  Bool weak_delete_called;
  Bool close_on_empty_buffer;

  // Important lock order = file, then file_list.
  static WeakReferenceList<FileHandle *> file_list;
  static void closeFiles();

protected:
  Bool watch_read;
  Bool watch_write;

  string out_buffer;

  virtual void sawEOF();
  virtual void error();
  virtual void spaceAvailable();
  virtual void dataAvailable();

  virtual int read(int fd, void *buff, size_t buff_size) {
    return ::read(fd, buff, buff_size);
  }

  virtual int write(int fd, const void *buff, size_t buff_size) {
    return ::write(fd, buff, buff_size);
  }

  int weakDelete()
  {
    assert(!textus::base::init::shutdown_done);
    close();
    return Handle::weakDelete();
  }

public:
  static FileHandle *openFile(string name, int access, int mode=0600);

  FileHandle(): fh(-1), eof_count(3),  non_blocking(true), close_on_exec(true), read_count(0), watch_zero_reads(false), weak_delete_called(false), close_on_empty_buffer(false), watch_read(false), watch_write(false)
{
#ifdef DEBUG_LOCKS
    lock_level =  1;
#endif
  }
  virtual ~FileHandle() {
    close();
    assert(!on_list);
    if (fh != -1) {
      ::close(fh);
      fh = -1;
    }
  }

  static Base *threadMonitor(Base *);
  static void wakeMonitor();
  static Base *execBarrierObject();
  static void shutdown();
  static void init();

  const int nativeHandle() {
    return fh;
  }

  void watchZeroReads(bool f) {
    watch_zero_reads = f;
  }

  void closeOnEmptyBuffer(bool f = true) {
    close_on_empty_buffer = f;
  }

  virtual void openNoLock(int f) {
    assert(haveSync());
    assert(fh < 0);
    fh = f;
    if (fh >= 0) {
      setNonBlocking(non_blocking);
      setCloseOnExec(close_on_exec);
      if (eventFactory()) {
	watch_read = true;
      }
    }
  }

  virtual void setEventFactory(HandleEventFactory *f) {
    Synchronized(this);
    Handle::setEventFactory(f);
    if (f != NULL && fh >= 0) {
      watch_read = true;
      wakeMonitor();
    }
  }

  virtual void canWrite(Event *e) {
    Synchronized(this);
    if (out_buffer.length() > 0) {
      watch_write = true;
      size_t ret = write(out_buffer);
      LOG(DEBUG) << "Attempted to write " << out_buffer.length() << " wrote " << ret << "bytes.\n";
      if (ret > 0) {
	out_buffer = out_buffer.substr(ret);
	if (out_buffer.length() == 0) {
	  watch_write = false;
	  if (close_on_empty_buffer) {
	    close();
	  }
	} else {
	  watch_write = true;
	}
      }
      wakeMonitor();
    }
    return;
  }

  virtual void addToOutBuffer(string d) {
    Synchronized(this);
    size_t l = out_buffer.length();
    out_buffer = out_buffer + d;
    if (l == 0) {
      canWrite(NULL);
    }
  }

  virtual void addToFileList() {
    assert(!textus::base::init::shutdown_done);
    Synchronized(this);
    Synchronized(&file_list);
    file_list.erase(this);
    on_list = false;
    if (fh >= 0) {
      file_list.push_back(this);
      on_list = true;
      wakeMonitor();
    }
  }

  virtual void open(int f) {
    assert(!textus::base::init::shutdown_done);
    Synchronized(this);
    Synchronized(&file_list);
    file_list.erase(this);
    if (fh != -1) {
      close();
    }
    EXEC_BARRIER(); // Caller should already have done this.
    openNoLock(f);

    if (fh >= 0) {
      file_list.push_back(this);
      wakeMonitor();
    }
  }

  virtual int write(string data) { 
    Synchronized(this);
    if (data.length() != 0)  {
      //XXXXX Fixme:  Valgrind claims this touches unitialized data.
      return write(fh, data.data(), data.length()); 
    } else {
      return 0;
    }
  }

  virtual string read(int maxlen);

  virtual off_t seek(off_t off, int whence) {
    return lseek(fh, off, whence);
  }

  virtual off_t tell() {
    return seek(0, SEEK_CUR);
  }

  bool nonBlocking() {
    Synchronized(this);
    return non_blocking;
  }

  bool closeOnExec() {
    Synchronized(this);
    return close_on_exec;
  }

  int setCloseOnExec(bool on) {
    Synchronized(this);
    close_on_exec = on;
    int arg = 0;
    if (fh >= 0) {
      if (close_on_exec) {
	arg = fcntl (fh, F_SETFD, FD_CLOEXEC);
      } else {
	arg = fcntl (fh, F_SETFD, 0);
      }
      if (arg < 0) {
	AUTODEREF(Error *, e);
	e = new Error();
	LOG(ERROR) << "fcntl F_SETFD: " << e;
	LOG_CALL_STACK(ERROR);
	die();
      }
    }
    return arg;
  }

  int setNonBlocking(bool on) {
    Synchronized(this);
    non_blocking = on;
    int arg = 0;
    if (fh >= 0) {
      arg = fcntl(fh, F_GETFL, NULL);
      if (arg < 0) {
	AUTODEREF(Error *, e);
	e = new Error();
	LOG(ERROR) << "fcntl F_GETFL: " << e;
	LOG_CALL_STACK(ERROR);
	die();
      }
      if (on) {
	arg = fcntl (fh, F_SETFL, arg | O_NONBLOCK);
      } else {
	arg = fcntl (fh, F_SETFL, arg & ~O_NONBLOCK);
      }
      if (arg < 0) {
	AUTODEREF(Error *, e);
	e = new Error();
	LOG(ERROR) << "fcntl F_SETFL: " << e;
	LOG_CALL_STACK(ERROR);
	die();
      }
    }

    return arg;
  }

  bool valid()
  { 
    Synchronized(this);
    return fh != -1; 
  }

  virtual void close()
  { 
    {
      assert(!textus::base::init::shutdown_done);
      Synchronized(this);
      Synchronized(&file_list);
      file_list.erase(this);
      on_list = false;
      if (fh != -1) {
	::close(fh);
	fh = -1;
      }
    }

    Handle::close();
  }

  virtual int tryReadLock() {
    Synchronized(this);
    int ret = ::flock(nativeHandle(), LOCK_SH|LOCK_NB);
    if (ret && (errno != EWOULDBLOCK && errno != EAGAIN)) {
      error();
      return false;
    }
    return ret == 0;
  }

  virtual int readLock() {
    Synchronized(this);
    int ret = ::flock(nativeHandle(), LOCK_SH);
    if (ret) {
      error();
      return false;
    }
    return ret == 0;
  }

  virtual int readUnlock() {
    Synchronized(this);
    int ret = ::flock(nativeHandle(), LOCK_UN);
    if (ret) {
      error();
      return false;
    }
    return ret == 0;
  }

  virtual void waitForClose() {
    Synchronized(this);
    while(nativeHandle() >= 0) {
      wait();
    }
  }

  virtual int tryWriteLock() {
    Synchronized(this);
    int ret = ::flock(nativeHandle(), LOCK_EX|LOCK_NB);
    if (ret && errno != EWOULDBLOCK && errno != EAGAIN) {
      error();
      return false;
    }
    return ret == 0;
  }

  virtual int writeLock() {
    Synchronized(this);
    int ret = ::flock(nativeHandle(), LOCK_EX);
    if (ret) {
      error();
      return false;
    }
    return ret == 0;
  }

  virtual int writeUnlock() {
    Synchronized(this);
    int ret = ::flock(nativeHandle(), LOCK_UN);
    if (ret) {
      error();
      return false;
    }
    return ret == 0;
  }

  virtual int flush() {
    return ::fsync(nativeHandle());
  }

};
}} //namespace

#endif // TEXTUS_FILE_FILE_HANDLE_H_
