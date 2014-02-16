/* Log.cc -*- c++ -*-
 * Copyright (c) 2009, 2014 Ross Biro
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

#include "textus/logging/Log.h"
#include "textus/logging/LoggerBuf.h"

#include <execinfo.h>
#include <cxxabi.h> 
#include <string.h>

namespace textus { namespace logging { 

FileLogger Log_DEBUG("DEBUG");
FileLogger Log_INFO("INFO");
FileLogger Log_WARNING("WARNING");
FileLogger Log_ERROR("ERROR");

static FileLogger *loggers[] = {
  &Log_DEBUG,
  &Log_INFO,
  &Log_WARNING,
  &Log_ERROR,
  NULL
};

void log_call_stack(FileLogger &log, const char *file, int line) 
{
  const size_t max_depth = 100;
  size_t stack_depth;
  void *stack_addrs[max_depth];
  char **stack_strings;

  stack_depth = backtrace(stack_addrs, max_depth);
  stack_strings = backtrace_symbols(stack_addrs, stack_depth);

  log << "Call stack from " << file << "(" << line << ")\n";

  for (size_t i = 1; i < stack_depth; i++) {
    size_t sz = 200; // just a guess, template names will go much wider
    char *function = static_cast<char *>(malloc(sz));
    char *begin = 0, *end = 0;
    // find the parentheses and address offset surrounding the mangled name
    for (char *j = stack_strings[i]; *j; ++j) {
      if (*j == '(') {
	begin = j;
      } else if (*j == '+') {
	end = j;
      }
    }

    if (begin && end) {
      *begin++ = 0;
      *end = 0;
      // found our mangled name, now in [begin, end)

      int status;
      char *ret = abi::__cxa_demangle(begin, function, &sz, &status);
      if (ret) {
	// return value may be a realloc() of the input
	function = ret;
      } else {
	// demangling failed, just pretend it's a C function with no args
	strncpy(function, begin, sz);
	strncat(function, "()", sz);
	function[sz-1] = 0;
      }
      log << stack_strings[i] << ":" << function << "\n";
    } else {
        // didn't find the mangled name, just print the whole line
      log << stack_strings[i] << "\n";
    }
    free(function);
  }

  free(stack_strings); // malloc()ed by backtrace_symbols
}

void dump_all_threads() 
{
  
}

FileLogger::FileLogger(const char *name)
{
  // XXXXXX Fixme: deal with the case that creating the LoggerBuf
  // fails. XXXXXX
  static LoggerBuf *previous = NULL;
  LoggerBuf *lb = new LoggerBuf(name);
  logger_buf = lb;
  init(lb);
  lb->setParent(previous);
  previous = lb;
}

FileLogger::~FileLogger()
{
  flush();
  init(NULL);
  if (logger_buf) {
    delete logger_buf;
  }
  logger_buf = NULL;

}

int FileLogger::rotate()
{
  if (logger_buf) {
    return logger_buf->rotate();
  }
  return -1;
}


// This will get called after all of the command line and config file
// arguments are processed.  We just reset everything incase something
// important changed.
DEFINE_INIT_FUNCTION(reset, EARLY_INIT_PRIORITY) {
  for (int i = 0; loggers[i] != NULL; ++i) {
    loggers[i]->rotate();
  }
  return 0;
}


}} //namespace
