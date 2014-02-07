/* FileHandle.cc -*- c++ -*-
 * Copyright (c) 2009-2013 Ross Biro
 *
 * Represents a system file handle.
 *
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/select.h>

#include "textus/file/FileHandle.h"
#include "textus/base/Thread.h"
#include "textus/base/logic/Bool.h"
#include "textus/event/Event.h"
#include "textus/file/FileHandleEventFactory.h"
#include "textus/base/Compiler.h"
#include "textus/file/SystemFile.h"

namespace textus { namespace file {

using namespace textus::base;
using namespace textus::event;

static Thread *file_monitor_thread(NULL);
static int max_used_fd = -1;

WeakReferenceList<FileHandle *>FileHandle::file_list;

static textus::base::logic::Bool threadDone = false;
static struct timeval timeout;
static textus::base::Mutex timeout_mutex;
static int pipe_fds[2];

Base *FileHandle::execBarrierObject() {
  static Base obj;
  return &obj;
}

void FileHandle::wakeMonitor() 
{
  if (file_monitor_thread) {
    ::write (pipe_fds[1], " ", 1);
  }
}

static void shutdownMonitor() 
{
  threadDone = true;

  {
    LOCK(&timeout_mutex);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
  }

  FileHandle::wakeMonitor();
  assert(file_monitor_thread != NULL);
  file_monitor_thread->join();
  file_monitor_thread->deref();
  file_monitor_thread = NULL;
  FileHandle::shutdown();
}

void FileHandle::shutdown() {
  FileHandle *fh;
  while (!file_list.empty()) {
    fh = *file_list.begin();
    fh->on_list = false;
    file_list.pop_front();
  }
}

FileHandle *FileHandle::openFile(string name, int access, int mode) {
  SystemFile *sf = new SystemFile();
  if (sf->open(name, access, mode) < 0) {
    delete sf;
    return NULL;
  }

  return sf;
}

Base *FileHandle::threadMonitor(Base *arg) 
{
  Thread::self()->setDoAll(false);
  {
    EXEC_BARRIER();
    if (pipe(pipe_fds) < 0) {
      LOG(ERROR) << "Unable to create pipe to wake file monitor: " << errno << "\n";
      die();
    }
    int arg = fcntl(pipe_fds[0], F_GETFL, NULL);
    if (arg < 0) {
      LOG(ERROR) << "Unable to call F_GETFL on file monitor pipe: " << errno << "\n";
      die();
    }
    arg |= O_NONBLOCK;
    arg = fcntl(pipe_fds[0], F_SETFL, arg);
    if (arg < 0) {
      LOG(ERROR) << "Unable to call F_GETFL on file monitor pipe: " << errno << "\n";
      die();
    }
    arg = fcntl (pipe_fds[0], F_SETFD, FD_CLOEXEC);
    if (arg < 0) {
      LOG(ERROR) << "Unable to call F_GETFL on file monitor pipe: " << errno << "\n";
      die();
    }

    arg = fcntl(pipe_fds[1], F_GETFL, NULL);
    if (arg < 0) {
      LOG(ERROR) << "Unable to call F_GETFL on file monitor pipe: " << errno << "\n";
      die();
    }
    arg |= O_NONBLOCK;
    arg = fcntl(pipe_fds[1], F_SETFL, arg);
    if (arg < 0) {
      LOG(ERROR) << "Unable to call F_GETFL on file monitor pipe: " << errno << "\n";
      die();
    }
    arg = fcntl(pipe_fds[1], F_SETFD, FD_CLOEXEC);
    if (arg < 0) {
      LOG(ERROR) << "Unable to call F_GETFL on file monitor pipe: " << errno << "\n";
      die();
    }
  }

  file_monitor_thread = Thread::self();
  file_monitor_thread->ref();

  if (arg != NULL) {
    LOG(ERROR) << "threadMonitor called with non-NULL argument\n";
    die();
  }
    
  while (!threadDone) {
    fd_set readfds, writefds, errorfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errorfds);
      
    ReferenceList<FileHandle *> active;
    {
      // Get a strong reference to everything, so it doesn't go away while we are mucking with it.
      Synchronized(&file_list);
      for (WeakReferenceList<FileHandle *>::iterator i = file_list.begin(); i != file_list.end(); ++i) {
	active.push_back(*i);
      }
    }

    int nfds=0;

    FD_SET(pipe_fds[0], &readfds);
    FD_SET(pipe_fds[0], &errorfds);
    nfds = pipe_fds[0]+1;
    assert(!textus::base::init::shutdown_done);

    for (ReferenceList<FileHandle *>::iterator i = active.begin(); i != active.end(); ++i) {
      FileHandle *fh = *i;
      Synchronized(fh);

      if (fh->eof_flag && !fh->sent_eof) {
	fh->sent_eof = true;
	fh->sawEOF();
	continue;
      }

      // Make sure it's not closed.
      if (fh->fh < 0) {
	continue;
      }

      if (fh->fh >= nfds) {
	nfds = fh->fh + 1;
      }

      FD_SET(fh->fh, &errorfds);
	  
      if (fh->watch_read) {
	FD_SET(fh->fh, &readfds);
      }

      if (fh->watch_write) {
	FD_SET(fh->fh, &writefds);
      }
    }

    max_used_fd = nfds - 1;

    // We need to be careful through here.  First we set the timeout
    // to infinite, then we make sure it's flushed to memory before we
    // check the threadDone.  threadDone is set before timeout is set
    // to 0.  If the timeout is set to 0 before the kernel reads it,
    // then we are good.  If the kernel reads it first, then we are in
    // the system call and the select will return eintr.  There is a
    // chance we will get an invalid timeout, but it's doubtful.  The
    // threadDone check before the select and after setting the
    // timeout is necessary.
    {
      LOCK(&timeout_mutex);
      timeout.tv_sec = 60*60*24;
      timeout.tv_usec = 0;
    }

    if (threadDone) {
      break;
    }

    // Since we can't keep a lock on all the file handles, it's possible someone
    // will close one on us.
    // Need to process thread events here, before waiting.
    while (Thread::self()->waitingTasks(NULL));
    nfds = select(nfds, &readfds, &writefds, &errorfds, &timeout);

    // EINTR is normal, we wake the thread up when something changes by sending a signal.
    // EBADF probably means someone closed a file handle on us when we weren't looking.
    if (nfds < 0) {
      LOG(DEBUG) << "select error : " << errno << "\n";
      if (errno != EINTR && errno != EBADF) {
	LOG(ERROR) << "select error : " << errno << "\n";
	die();
      }
    }

    if (threadDone) {
      break;
    }

    if (nfds > 0) {
      char buff[10];
      while (::read(pipe_fds[0], buff, sizeof(buff)) > 0);
      {
	assert(!textus::base::init::shutdown_done);
	for (ReferenceList<FileHandle *>::iterator i = active.begin(); i != active.end(); ++i) {
	  FileHandle *fh = *i;
	  assert(fh != NULL);
	  Synchronized(fh);
	  // make sure no one closed this one on us since we last saw it.
	  if (fh->fh < 0) {
	    continue;
	  }
	  if (FD_ISSET(fh->fh, &errorfds) ||
	      FD_ISSET(fh->fh, &readfds) ||
	      FD_ISSET(fh->fh, &writefds)) {
	    active.push_front(fh);
	  }
	}
      }

      for (ReferenceList<FileHandle *>::iterator i = active.begin(); i != active.end(); ++i) {
	FileHandle *fh = *i;
	Synchronized(fh);
	if (fh->fh == -1) {
	  continue;
	}

	assert(fh->fh != -1);

	if (FD_ISSET(fh->fh, &readfds) && fh->watch_read) {
	  fh->dataAvailable();
	}

	if (FD_ISSET(fh->fh, &errorfds)) {
	  fh->error();
	}

	if (FD_ISSET(fh->fh, &writefds) && fh->watch_write) {
	  fh->spaceAvailable();
	}
      }
    }
    active.clear(); // this will give up our reference to the handle, possibly, trigger it's deletion.
  }
  return NULL;
}

INITTHREAD(FileHandle::threadMonitor, NULL);
SHUTDOWNTHREAD(shutdownMonitor);

void FileHandle::init() {
#ifdef DEBUG_LOCKS
    file_list.setLockLevel(1);
#endif
}

DEFINE_INIT_FUNCTION(fhInit, EARLY_INIT_PRIORITY)
{
  FileHandle::init();
  return 0;
}

string FileHandle::read(int maxlen) {
  Synchronized(this);

  if (fh == -1) {
    watch_read = false;
    eof_flag = true;
    error_flag = true;
    LOG(DEBUG) << "Inavlid file handle in read.\n";
    return "";
  }

  AUTOFREE(void *, buff);
  buff = malloc(maxlen);

  if (buff == NULL) {
    LOG(ERROR) << "Out of memory allocating read buffer.\n";
    die();
    return string("");
  }

  int red = read(fh, buff, maxlen);

  if (red == 0) {
    if (eventFactory()) {
      watch_read = true;
    }
    eof_flag = true;
    return string("");
  }

  if (red < 0) {
    if (errno != EAGAIN) {
      watch_read = false;
      eof_flag = true;
      LOG(ERROR) << "error " << errno << "\n";
      error_flag = true;
      err = errno;
      return string("");
    }
    return string("");
  }

  read_count += red;

  return string((const char *)buff, (string::size_type)red);
}

void FileHandle::sawEOF()
{
  Synchronized(this);
  if (eventFactory()) {
    AUTODEREF(Event *, ev);
    ev = eventFactory()->eofEvent();
    if (ev) {
      if (ev->preferredQueue() || !eventQueueScheduler()) {
	ev->post();
      } else {
	ev->post(eventQueueScheduler()->getQueue());
      }
    }
  }
}

void FileHandle::error()
{
  Synchronized(this);
  error_flag = true;
  dataAvailable();
}

// Means we can write some more.
void FileHandle::spaceAvailable()
{
  Synchronized(this);
  AUTODEREF(Event *, ev);
  if (eventFactory()) {
    ev = eventFactory()->writeEvent();
    if (ev) {
      if (ev->preferredQueue() || !eventQueueScheduler()) {
	ev->post();
      } else {
	ev->post(eventQueueScheduler()->getQueue());
      }
    }
  }
  watch_write = false;
}

// Means we can read some more.
void FileHandle::dataAvailable()
{
  Synchronized(this);
  static const int buffer_size=1024;
  if (eventFactory()) {
    for (string s = read(buffer_size); s != ""; s=read(buffer_size)) {
      AUTODEREF(Event *, ev);
      ev = eventFactory()->readEvent(s);
      if (ev) {
	if (ev->preferredQueue() || !eventQueueScheduler()) {
	  ev->post();
	} else {
	  ev->post(eventQueueScheduler()->getQueue());
	}
      } else {
	LOG(ERROR) << "Unable to create event.\n";
	die();
      }
    }
  }
}

}} //namespace
