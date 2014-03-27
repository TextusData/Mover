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
#ifndef TEXTUS_TEMPLATE_TEMPLATE_H_
#define TEXTUS_TEMPLATE_TEMPLATE_H_

#include <string>
#include <set>

#include "textus/base/Base.h"
#include "textus/base/OneTimeUse.h"
#include "textus/file/FileBackedNumber.h"
#include "textus/file/FileHandle.h"
#include "textus/base/functor/Functor.h"

namespace textus { namespace template_ {

using namespace textus::base;

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
  size_t process(const std::string &in, std::string *out);

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

template <class T> class TemplateFunctor:
    public textus::base::functor::StringFunctor {
 private:
  AutoDeref< Template<T> > templ;

 public:
  explicit TemplateFunctor(Template<T> *t): templ(t) {}
  virtual ~TemplateFunctor() {}
  virtual int apply(const string &in, string *out) {
    int ret = 0;
    HRC(templ->process(in, out));
  error_out:
    return ret;
  }
};

}} //namespace

#endif //TEXTUS_TEMPLATE_TEMPLATE_H_
