/* Log.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Routines to facilitate logging.
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

#ifndef TEXTUS_LOGGING_LOG_H_
#define TEXTUS_LOGGING_LOG_H_

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <ostream>
#include <string>
#include <list>

// These need to go up here because we use them in a lot of the includes.
namespace textus { namespace logging {
using namespace std;

class LoggerBuf;

class FileLogger: public ostream  {
private:
  LoggerBuf *logger_buf;

public:
  static const long max_log_file_size = 40*1024*1024;
  static const long max_log_file_lines = 100000;
  FileLogger(const char *);
  ~FileLogger();

  int rotate();
};

void log_call_stack(FileLogger &, const char *file, int line);

extern FileLogger Log_ERROR;
extern FileLogger Log_WARNING;
extern FileLogger Log_INFO;
extern FileLogger Log_DEBUG;

void dump_all_threads(void);

}} // namespace

#define LOG(type) textus::logging::Log_ ##type << __FILE__ << "-" << __LINE__ << ":"
#define LOG_CALL_STACK(type) textus::logging::log_call_stack(textus::logging::Log_##type, __FILE__, __LINE__)

static inline void die(void) {
  LOG(ERROR) << "Die Called\n";
  LOG_CALL_STACK(ERROR);
  textus::logging::dump_all_threads();

  // emulate a segment violation to try to get a core dump.
  kill(getpid(), SIGSEGV);

  // If that didn't work, just segfault.
  *(unsigned long *)0 = 0;

  // If that still didn't work, just exit.
  exit(127);

}

#endif //TEXTUS_LOGGING_LOG_H_
