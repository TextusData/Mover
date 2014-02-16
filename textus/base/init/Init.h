/* Init.h -*- c++ -*-
 * Copyright (C) 2009-2010, 2013, 2014 Ross Biro
 *
 * Initialization (system startup) functions and macros
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

#ifndef TEXTUS_BASE_INIT_INIT_H_
#define TEXTUS_BASE_INIT_INIT_H_

#include "textus/base/Base.h"
#include "textus/base/logic/Bool.h"
#include <list>
#include <vector>
#include <map>
#include <string>

namespace textus { namespace file {
class Directory;
}} // namespace

namespace textus { namespace base { namespace init {
using namespace std;

extern volatile bool shutdown_done;
extern volatile bool shutdown_started;

typedef int (*initialization_function)(list<string *> args);
struct initializer_array_element {
  initialization_function f;
  int p;
};

typedef void (*shutdown_function)(void);
struct shutdown_array_element {
  shutdown_function f;
  int p;
};

class InitializerAppender {
private:
  static vector<initializer_array_element> &array();

public:
  static int ExecuteFunctions(list<string *> args);
  static void clear();

  InitializerAppender(initialization_function function, int priority) {
    initializer_array_element element;
    element.f = function;
    element.p = priority;
    array().push_back(element);
  }
};

class ShutdownAppender {
private:
  static vector<shutdown_array_element> &array();
public:
  static void ExecuteFunctions(void);
  static void clear();

  ShutdownAppender(shutdown_function function, int priority) {
    struct shutdown_array_element element;
    element.f = function;
    element.p = priority;
    array().push_back(element);
  }
};

class ArgumentAppender {
public:
  typedef int (*converter_func)(const string &, void *);

private:
  struct initializer {
    void *arg;
    converter_func func;
    const char * description;
  } *i;

  static map<string, initializer *> &argument_map();

public:
  static int processOneArgString(string arg);
  static int processOneArgument(string arg, string value);
  static int processArguments(list<string *> *args);
  static int parseOptionsFile(string path, bool ignore_missing = false);
  static void usage();
  static void clear();
  static int setArg(string s);
  static int setArg(string s, string v);
  static int setArg(string s, long v);

  ArgumentAppender(void *a, converter_func f, const char * const arg,
		   const char * const desc) 
  {
    i = static_cast<initializer *>(malloc(sizeof(*i)));
    if (i == NULL) {
      LOG(ERROR) << "Out of Memory while processing parameters.\n";
      die();
    }
    i->arg = a;
    i->func = f;
    i->description = desc;

    string s = arg;
    
    argument_map()[s] = i;
  }

  ~ArgumentAppender() {
    free(i);
  }
};

int converter_int(const string &, void *);
int converter_unsigned(const string &, void *);
int converter_int64_t(const string &, void *);
int converter_string(const string &, void *);
int converter_Bool(const string &, void *);
int converter_double(const string &, void *);
int converter_protobuf(const string &, void *);
  
const string &getAppName(void);

const textus::file::Directory &getAppDir();

typedef map<string, string> env_map;
const env_map &initEnvironment();
string getConfigPath(const string file);
string getDataPath(const string file);

void TextusInit(int argc, const char *argv[], const char *envp[]);
void TextusShutdown();
const list<string *> &getCommandLine();

template <class C, ArgumentAppender::converter_func f >
class list_args:public std::list<C> {
public:
  explicit list_args(const char *test_args, const char *c) {
    add_to_args(textus::base::testing?std::string(test_args):std::string(c));
  }

  explicit list_args(std::string c) {
    add_to_args(c);
  }

  virtual ~list_args() {}

  int add_to_args(std::string s) {
    int ret = 0;
    if (s == "") {
      return ret;
    }
  
    size_t c = s.find_first_of(",");
    while (c != std::string::npos) {
      C data;
      ret = f(s.substr(0,c), &data);
      if (ret != 0) {
	return ret;
      }
      list<C>::push_back(data);
      s = s.substr(c+1);
      while (s.length() > 0 && isspace(s[0])) {
	s = s.substr(1);
      }
      c = s.find_first_of(",");
    }
    C data;
    ret = f(s, &data);
    if (ret != 0) {
      return ret;
    }
    list<C>::push_back(data);
    return 0;
  }

};

template <class T> inline int list_converter(const string &, void *) {
  LOG(ERROR) << "Unspecialized list converter called.\n";
  return -1;
}

template <> inline int list_converter<string>(const string &c, void *l) {
  list_args<string, converter_string> *la = reinterpret_cast<list_args<string, converter_string> * >(l);
  assert(la != NULL);
  return la->add_to_args(c);
}

template <typename T> int clear_list_arg(const string &s, void *vptr) {
  T *l = static_cast<T *>(vptr);
  if (s.length() > 0) {
    return -1;
  }
  l->clear();
  return 0;
}

template <class C> class ValueInitializerBase {
public:
  explicit ValueInitializerBase() {}
  virtual ~ValueInitializerBase() {}
  virtual std::string name() = 0;
  virtual std::string getValue(C *th) = 0;
  virtual int setValue(C *th, const std::string &value) = 0;
};

template <class C> class ValueHeaders {
private:
  typedef map<std::string, ValueInitializerBase<C> > MapType;
  MapType values;

public:
  explicit ValueHeaders() {}
  ~ValueHeaders() {}
  
  void addValue(ValueInitializerBase<C> *vib) {
    values[vib.name()] = vib;
  }

  void eraseValue(ValueInitializerBase<C> *vib) {
    values.erase(vib.name());
  }

  std::string getValue(C *th, const std::string &key) {
    class MapType::iterator it = values.find(key);
    if (it == values.end()) {
      return "";
    }
    ValueInitializerBase<C> *vib = *it;
    return vib->getValue(th);
  }

  int setValue(C *th, const std::string &key, const std::string &value) {
    class MapType::iterator it = values.find(key);
    if (it == values.end()) {
      return 1;
    }
    ValueInitializerBase<C> *vib = *it;
    return vib->setValue(th, value);
  }
};


template <class C, class T, int (*converter)(const std::string &s, void *v), std::string (*formatter)(T) >
	  class ValueInitializer: public ValueInitializerBase<C> 
{
 private:
  typedef T C::*MemberPtr;
  std::string des;
  std::string nam;
  MemberPtr ptr;

 public:
  explicit ValueInitializer(std::string n, std::string def, std::string description, MemberPtr p):des(description), nam(n), ptr(p)  {
    C::value_headers.addValue(this);
  }

  virtual ~ValueInitializer() {
    C::value_headers.eraseValue(this);
  }

  virtual std::string name() {
    return nam;
  }

  virtual std::string getValue(C *th) {
    return formatter(th->*ptr);
  }

  virtual int setValue(C *th, const std::string &value) {
    void *v = static_cast<void *>(&th->*ptr);
    return converter(value, v);
  }

};


}}} //namespace

#define DECLARE_INIT_FUNCTION(name) int name(list<string *> args)
#define ADD_TO_INIT_ARRAY(name, priority) static textus::base::init::InitializerAppender \
  UNIQUE_IDENTIFIER(INIT_APPENDER)(name, priority);
#define DEFINE_INIT_FUNCTION(name, priority) DECLARE_INIT_FUNCTION(name); ADD_TO_INIT_ARRAY(name, priority); \
  int name(list<string *> args)
#define REQUIRE_INIT_FUNCTION(name) extern DECLARE_INIT_FUNCTION(name)

#define DECLARE_SHUTDOWN_FUNCTION(name) void name ()
#define ADD_TO_SHUTDOWN_ARRAY(name, priority) static textus::base::init::ShutdownAppender \
  UNIQUE_IDENTIFIER(SHUTDOWN_APPENDER)( name, priority )
#define DEFINE_SHUTDOWN_FUNCTION(name, priority) DECLARE_SHUTDOWN_FUNCTION(name); ADD_TO_SHUTDOWN_ARRAY(name, priority); void name (void)


#define ADD_TO_ARGUMENT_ARRAY(var, converter, argument, description) static textus::base::init::ArgumentAppender \
  UNIQUE_IDENTIFIER(ARGUMENT)(var, converter, argument, description)


#define DECLARE_VALUE_METHODS(c)				\
  string getValue(const string &key) {				\
    return value_headers.getValue(this, key);			\
  }								\
  int setValue(const string &key, const string &value) {	\
    return value_headers.setValue(this, key, value);		\
  }

#define DECLARE_VALUE(type, name) type name;
#define DECLARE_VALUE_HEADER(c) static textus::base::init::ValueHeaders<c> value_headers;

#define DEFINE_VALUE_HEADER(c) ValueHeaders<c> c::value_headers;
#define DEFINE_VALUE(c, type, name, description) static textus::base::init::ValueInitializer<c, type, textus::base::init::convert_##type, textus::base::init::formatter_##type >(name, def, description, &c::name)

#define DECLARE_STRING_VALUE(name) DECLARE_VALUE(string, name)
#define DEFINE_STRING_VALUE(c, name, description) DEFINE_VALUE(c, string, name, description)

#define DECLARE_ARG(type, name) extern type name;
#define DEFINE_ARG(type, name, def, test_def, argument, description) \
  type name(textus::base::testing?test_def:def);		     \
  ADD_TO_ARGUMENT_ARRAY(&name, textus::base::init::converter_##type, \
			argument, description)

#define DECLARE_LIST_ARG(type, name) \
  extern textus::base::init::list_args<type, textus::base::init::converter_##type> name;
#define DEFINE_LIST_CLEAR_ARG(name, argument, type)			\
  DEFINE_ACTION_ARG(argument "_clear",					\
		    textus::base::init::clear_list_arg<type>,		\
		    &name, "Clear the list " argument ".");
#define DEFINE_LIST_ARG(type, name, argument, description, args) \
  textus::base::init::list_args<type, textus::base::init::converter_##type> name(args, args); \
  ADD_TO_ARGUMENT_ARRAY(&name,						\
  			textus::base::init::list_converter<type>,	\
  			argument, description);				\
  DEFINE_LIST_CLEAR_ARG(name, argument, typeof(name));

#define DEFINE_LIST_ARG_TEST(type, name, argument, description, test_def, def) \
  textus::base::init::list_args<type, textus::base::init::converter_##type> name(test_def, def ); \
  ADD_TO_ARGUMENT_ARRAY(&name,						\
  			textus::base::init::list_converter<type>,	\
  			argument, description);				\
  DEFINE_LIST_CLEAR_ARG(name, argument, typeof(name));

#define DECLARE_CUSTOM_ARG(type, name) DECLARE_ARG(type, name)
#define DEFINE_CUSTOM_ARG(type, name, def, converter, argument, description) \
  type name(def);							\
  ADD_TO_ARGUMENT_ARRAY(&name, converter, argument, description)

#define DECLARE_INT_ARG(name) DECLARE_ARG(int, name)
#define DEFINE_INT_ARG(name, def, argument, description) \
  DEFINE_ARG(int, name, def, def, argument, description)

#define DECLARE_UINT_ARG(name) DECLARE_ARG(unsigned, name)
#define DEFINE_UINT_ARG(name, def, argument, description) \
  DEFINE_ARG(unsigned, name, def, def, argument, description)

#define DECLARE_INT64_ARG(name) DECLARE_ARG(int64_t, name)
#define DEFINE_INT64_ARG(name, def, argument, description) \
  DEFINE_ARG(int64_t, name, def, def, argument, description)

#define DECLARE_STRING_ARG(name) DECLARE_ARG(string, name)
#define DEFINE_STRING_ARG(name, def, argument, description) \
  DEFINE_ARG(string, name, def, def, argument, description)

#define DECLARE_DOUBLE_ARG(name) DECLARE_ARG(double, name)
#define DEFINE_DOUBLE_ARG(name, def, argument, description) \
  DEFINE_ARG(double, name, def, def, argument, description)

#define DECLARE_BOOL_ARG(name) DECLARE_ARG(textus::base::logic::Bool, name)
#define DEFINE_BOOL_ARG(name, def, argument, description) \
  DEFINE_CUSTOM_ARG(textus::base::logic::Bool, name, def, textus::base::init::converter_Bool, argument, description)

#define DECLARE_PROTOBUF_ARG(name) DECLARE_ARG(std::string, name)
#define DEFINE_PROTOBUF_ARG(name, def, argument, description) \
  DEFINE_CUSTOM_ARG(std::string, name, def, textus::base::init::converter_protobuf, argument, description)

#define DEFINE_ACTION_ARG(argument, action, vptr, description)		\
  ADD_TO_ARGUMENT_ARRAY(vptr, action, argument, description)

#define EARLY_INIT_PRIORITY 100
#define LATE_INIT_PRIORITY 1000000

#endif // TEXTUS_BASE_INIT_INIT_H_
