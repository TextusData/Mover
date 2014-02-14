/* LoggerBuf.cc -*- c++ -*-
 * Copyright (C) 2009-2013 Ross Biro
 *
 * The internal workings of the logger class.
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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string>
#include <ios>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

#include "textus/logging/LoggerBuf.h"
#include "textus/base/Thread.h"
#include "textus/base/init/Init.h"

namespace textus { namespace logging {
using namespace std;

DEFINE_BOOL_ARG(log_to_stderr, false, "log_to_stderr",  \
"Enable logging to stderr instread of logging to a file.\n");

DEFINE_STRING_ARG(log_time_format, "%F %T", "log_time_format", \
"Default time format used for logging and anything else that " \
"does a << struct timeval.");

DEFINE_STRING_ARG(log_directory, "/data/logs", "log_directory", 	\
"Directory to write log files into.  If it does not exist, it will be " \
		  "created with mkdir -p.");

static textus::base::Base syncObject;

LoggerBuf::LoggerBuf(const char *name):
  buffer(),
  output_stream(NULL),
  bytes_since_last_rotate(0),
  lines_since_last_rotate(0),
  log_name(name)
{
  // This is initialized at static init time.  That's too early.
}

LoggerBuf::~LoggerBuf()
{
  // Cleanup any buffers left over before deleting the thread local storage.
}

bool LoggerBuf::needRotate()
{
  Synchronized(&syncObject);
  if (output_stream == NULL || !output_stream->good())
    return true;

  if (bytes_since_last_rotate >= FileLogger::max_log_file_size)
    return true;

  if (lines_since_last_rotate >= FileLogger::max_log_file_lines) {
    return true;
  }

  return false;
}

string LoggerBuf::generateFileName()
{
  int pid = getpid();
  struct timeval now;
  gettimeofday(&now, NULL);
  char buf[40];
  snprintf (buf, sizeof(buf), "_%d_%ld.%06ld", pid, (long)now.tv_sec, (long)now.tv_usec);
  string fn = base::init::getAppName();
  fn += "_";
  fn += log_name;
  fn += buf;
  return fn;
}

int LoggerBuf::rotate()
{
  Synchronized(&syncObject);
  bytes_since_last_rotate = 0;
  lines_since_last_rotate = 0;

  if (log_to_stderr) {
    parent = NULL;
    output_stream = &std::cerr;
    return 0;
  } 

  if (output_stream && output_stream != &std::cerr) {
    output_stream->flush();
    delete output_stream;
  }

  output_stream = NULL;

  struct stat stat_buf;
  if (stat(log_directory.c_str(), &stat_buf)) {
    if (errno == ENOENT) {
      string command = string("mkdir -p '") + log_directory + "'";
      system(command.c_str());
    } else {
      output_stream = &std::cerr;
      return -1;
    }
  }

  string path = log_directory + "/" + generateFileName();
  output_stream = new ofstream(path.c_str());

  return 0;
}

ostream &operator << (ostream &stream, struct timeval &tv)
{
  char buf[100];
  struct tm tm;
  time_t tt = tv.tv_sec;
  gmtime_r(&tt, &tm);
  strftime(buf, sizeof(buf), log_time_format.c_str(), &tm);
  return stream << "GMT: " << buf << "." 
		<< setw(6) << setfill('0') << tv.tv_usec;
}

// puts critical information at the start of the line.
void LoggerBuf::lineID()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  textus::base::Thread *th = textus::base::Thread::self();
  if (th == NULL) {
    *output_stream << now << "-NULL:\t:";
  } else {
    *output_stream << now << "-" << th << ":\t";
  }
}

string LoggerBuf::writeString(string str)
{
  Synchronized(&syncObject);

  if (parent) {
    parent->writeString(str);
  }

  if (needRotate()) {
    rotate();
  }

  for (string::size_type eol = str.find('\n');
       eol != string::npos;
       eol = str.find('\n')) {
    lineID();
    *output_stream << str.substr(0, eol + 1);
    str = str.substr(eol+1);
  }
  output_stream->flush();
  return str;
}

streamsize LoggerBuf::xsputn(const char *s, streamsize n)
{
  // XXXXXX FIXME: the string associated with the stringbuf
  // never seems to get freed.
  stringbuf *sb = static_cast<stringbuf *>(buffer);
  if (sb == NULL) {
    // Thread local, no locks necessary.
    sb = new stringbuf();
    buffer = sb;
    if (sb == NULL) {
      // Can't log an error, we are in the error logger now.
      die();
    }
  }
  //XXXXX FIXME: valgrind thinks this is a leak. See Supression LoggerBug xsputn
  streamsize  ret = sb->sputn(s, n);
  string str = sb->str();
  string::size_type eol = str.find('\n');
  if (eol != string::npos) {
    sb->str(writeString(str));
  }
  return ret;
}


}} // namespace
