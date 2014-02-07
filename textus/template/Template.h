/* Template.h -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
 *
 * A simple template language for strings.
 * replace <var> with the contents of var.
 * replace <?template:string> with string if
 * template is a non-empty string.
 * <[non-alpha-numeric]...> are all reserved for
 * future expansion.
 */
#ifndef TEXTUS_TEMPLATE_TEMPLATE_H_
#define TEXTUS_TEMPLATE_TEMPLATE_H_

#include <string>
#include <set>


#include "textus/base/Base.h"
#include "textus/base/OneTimeUse.h"
#include "textus/file/FileBackedNumber.h"
#include "textus/file/FileHandle.h"

namespace textus { namespace template_ {

class TemplateBase: virtual public textus::base::Base {
private:
  textus::base::OneTimeUse<textus::file::FileBackedNumber, uint64_t> counter;

protected:
  virtual int get(std::string key, std::string *value) = 0;
  virtual int evalConditional(std::string in, std::string *out);
  virtual int evalNumTemplate(std::string in, std::string *out);
  virtual uint64_t getNextNum() {
    return counter;
  }
public:
  explicit TemplateBase() {}
  virtual ~TemplateBase() {}
  size_t process(std::string in, std::string *out);
  int setBackingFile(std::string name) {
    int ret=0;
    HRC(counter.getBacking().attachFile(name));
  error_out:
    return ret;
  }

  int setBackingFile(textus::file::FileHandle *fh) {
    int ret=0;
    HRC(counter.getBacking().attachFile(fh));
  error_out:
    return ret;
  }

  std::set<std::string> findKeyWords(std::string &temp);
  
};

// T is the type of the map.
template <typename T> class Template: virtual public TemplateBase {
private:
  T data;
protected:
  virtual int get(std::string key, std::string *value) {
    if (data->count(key) == 0) {
      return -1;
    }
    return process(data->at(key), value);
  }

public:
  explicit Template(T d): data(d) {}
  virtual ~Template() {}

};

}} //namespace

#endif //TEXTUS_TEMPLATE_TEMPLATE_H_
