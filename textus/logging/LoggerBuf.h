/* LoggerBuf.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * The internal workings of the logging routines.
 *
 */

#ifndef TEXTUS_LOGGING_LOGGER_BUF_H_
#define TEXTUS_LOGGING_LOGGER_BUF_H_

#include "textus/base/ThreadLocal.h"
#include "textus/base/locks/Mutex.h"

namespace textus { namespace logging {
using namespace std;

class LoggerBuf: public streambuf {
private:
  textus::base::ThreadLocal<stringbuf *> buffer;
  ostream *output_stream;
  long bytes_since_last_rotate;
  long lines_since_last_rotate;
  const char *log_name;
  LoggerBuf *parent;

  string writeString(string s);
  void lineID();
  string generateFileName();

protected:
  streamsize  xsputn(const char *s, streamsize n);

public:
  explicit LoggerBuf(const char *name);
  virtual ~LoggerBuf();
  int rotate();
  bool needRotate();
  void setParent(LoggerBuf *lb) {
    parent = lb;
  }
  
};

ostream &operator << (ostream &stream, struct timeval &tv);

}}// namespace

#endif //TEXTUS_LOGGING_LOG_BUFFER_H_
