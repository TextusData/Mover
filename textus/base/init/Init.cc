/* Init.cc -*- c++ -*-
 * Copyright (C) 2009-2014 Ross Biro
 *
 * Initializion routines.
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
#include <stdlib.h>

#include <iostream>
#include <algorithm>
#include <sstream>

#include "textus/base/init/Init.h"
#include "textus/file/Directory.h"
#include "textus/file/SystemDirectory.h"
#include "textus/system/SysInfo.h"
#include "textus/file/TextusFile.h"
#include "textus/util/Base64.h"
#include "textus/util/Hex.h"
#include "textus/config/Config.h"


namespace textus { namespace base { namespace init {

#define MAX_PROTOBUF_ARG_SIZE 65536

using namespace std;

volatile bool shutdown_done = false;
volatile bool shutdown_started = false;

vector<struct shutdown_array_element> &ShutdownAppender::array() 
{
  static vector<struct shutdown_array_element> static_array;
  return static_array;
}

vector<initializer_array_element> &InitializerAppender::array()
{
  static vector<initializer_array_element> static_array;
  return static_array;
}

map<string, ArgumentAppender::initializer *> &ArgumentAppender::argument_map()
{
  static map<string, ArgumentAppender::initializer *> static_argument_map;
  return static_argument_map;
}


static list<string *> command_line;

static string app_name;
static string app_path;
static AutoDeref<textus::file::SystemDirectory> app_directory;

static env_map init_env;

const list<string *> &getCommandLine() {
  return command_line;
}

const string &getAppName() {
  return app_name;
}

const textus::file::Directory &getAppDir() {
  return *app_directory;  
}

const env_map &initEnvironment() {
  return init_env;
}

void ArgumentAppender::clear() {
  argument_map().clear();
}

void ShutdownAppender::clear() {
  array().clear();
}

void InitializerAppender::clear() {
  array().clear();
}

void TextusInit(int argc, const char *argv[], const char *envp[])
{

  // argv[0] is the prgram name, sometimes it has a complete path,
  // sometimes it does not.
  // XXXXX Fixme: There looks like leaks to valgrind
  app_path = argv[0];
  app_name = argv[0];
  shutdown_done = false;
  string::size_type last_slash = app_name.rfind('/');

  if (last_slash == string::npos) {
    // Try a system specific way to get the app path.
    string osapp = textus::system::SysInfo::systemInformation()->getAppPath();
    last_slash = osapp.rfind('/');
    if (last_slash != string::npos) {
      app_path = osapp;
      app_name = osapp;
    }
  }

  string path;
  if (last_slash != string::npos) {
    path = app_name.substr(0, last_slash);
    app_name = app_name.substr(last_slash + 1);
  } else {
    path = ".";
  }

  if (init_env.count("PATH")) {
    init_env["PATH"] = path + ":" + init_env["PATH"];
  } else {
    init_env["PATH"] = path;
  }
  setenv("PATH", path.c_str(), 1);
  init_env["APP_BINARY_PATH"] = path;

  string::size_type bin = app_path.rfind("/bin/");
  string app_dir;
  if (bin != string::npos) {
    // XXXXXX FXIME: valgrind thinks this is a leak supression: Init App Path
    string s = string("APP_PATH");
    app_dir = app_path.substr(0, bin);
    AUTODEREF(textus::file::SystemDirectory *, ad);
    ad = new textus::file::SystemDirectory(app_dir);
    app_directory = ad;
    init_env[s] = app_dir;
  } else {
    // XXXXX Fixme give unittest a way to prevent this message.
    LOG(WARNING) << "No app directory, probable configuration file problems.\n";
    init_env[string("APP_PATH")] = string("/");
  }

  for (int i = 1; i < argc; i++ ) {
    string *s = new string(argv[i]);
    char buff[50];
    snprintf(buff, sizeof(buff), "arg%d", i);
    init_env[buff] = *s;
    command_line.push_back(s);
  }

  for (int i = 0; envp[i] != NULL; i++) {
    // envp[i] is of the form foo=bar.
    string e(envp[i]);
    string::size_type s = e.find('=');
    if (s == string::npos) {
      init_env[e] = string("");
    } else {
      // XXXXXX FIXME: valgrind thinks this is a memory leak. Supression:Init init_env
      init_env[e.substr(0, s)] = e.substr(s+1);
    }
  }

  if (ArgumentAppender::parseOptionsFile(app_dir+"/options.def", true)) {
    exit(1);
  }

  if (ArgumentAppender::processArguments(&command_line)) {
    exit (1);
  }

  if (InitializerAppender::ExecuteFunctions(command_line)) {
    exit (1);
  }
}

void TextusShutdown(void)
{
  shutdown_started = true;
  ShutdownAppender::ExecuteFunctions();
  app_directory = NULL;
  for (list<string *>::iterator i = command_line.begin();
       i != command_line.end();
       ++i) {
    delete *i;
  }
  command_line.clear();
}

static bool initializerSortPredicate(const initializer_array_element& i1, const initializer_array_element& i2)
{
  return i1.p < i2.p;
}

static bool shutdownSortPredicate(const shutdown_array_element& s1, const shutdown_array_element& s2)
{
  return s1.p > s2.p;
}

int InitializerAppender::ExecuteFunctions(list<string *> args) 
{
  std::sort(array().begin(), array().end(), initializerSortPredicate);
  for (vector<initializer_array_element>::iterator i = 
	 InitializerAppender::array().begin();
       i != InitializerAppender::array().end(); ++i) {
    int ret = (i->f)(args);
    if (ret != 0)
      return ret;
  }
  return 0;
}

void ShutdownAppender::ExecuteFunctions(void) 
{
  // This needs to go here.  For some reason, putting it after the
  // cleanup functions triggers a race.
  init_env.clear();
  std::sort(array().begin(), array().end(), shutdownSortPredicate);
  for (vector<shutdown_array_element>::iterator i = ShutdownAppender::array().begin();
       i != ShutdownAppender::array().end(); ++i) {
    (i->f)();
  }
  ArgumentAppender::clear();
  ShutdownAppender::clear();
  InitializerAppender::clear();
  shutdown_done = true;
}

int ArgumentAppender::setArg(string s) {
  list<string *> a;
  string *t = new string(s);
  a.push_back(t);
  return processArguments(&a);
}


int ArgumentAppender::setArg(string s, string v) {
  return setArg(s+"="+v);
}

int ArgumentAppender::setArg(string s, long v) {
  char buff[14];
  snprintf(buff, sizeof(buff), "=%ld", v);
  return setArg(s+buff);
}

void ArgumentAppender::usage()
{
  cerr << "Usage: " << getAppName() << "\n";
  for (map<string, initializer *>::iterator map_iterator = 
	 argument_map().begin();
       map_iterator != argument_map().end();
       ++map_iterator)
    {
      initializer *i = (*map_iterator).second;
      cerr << (*map_iterator).first << "\t" << i->description << "\n";
    }
}

int ArgumentAppender::parseOptionsFile(string path, bool ignore_missing) {
  int ret = 0;
  AUTODEREF(textus::file::TextusFile *, tf);
  AUTODEREF(textus::config::Config *, config);
  tf = textus::file::TextusFile::openFile(path, O_RDONLY);
  if (tf == NULL) {
    if (ignore_missing) {
      LOG(INFO) << "option file " << path << " missing (ignored).\n";
      goto error_out;
    }
    LOG(INFO) << "option file " << path << " missing.\n";
    HRNULL(tf);
  }
  LOG(INFO) << "reading option file " << path << ".\n";
  config = new textus::config::Config();
  HRNULL(config);
  HRC(config->readFile(tf));
  
  textus::config::ConfigData *cd;
  cd = config->root();
  for (ReferenceValueMap<string, textus::config::ConfigData *>::iterator i =
	 cd->asMap().begin();
       i != cd->asMap().end();
       ++i) {
    HRC(processOneArgument(i->first, i->second->asString()));
  }
  
 error_out:
  return ret;
}

int ArgumentAppender::processOneArgument(string arg, string rest_arg) {

  if (textus::base::testing) {
    if (arg == "options-file-testing" && rest_arg != "") {
      return parseOptionsFile(rest_arg);
    }
  }

  if (arg == "options-file" && rest_arg != "") {
    return parseOptionsFile(rest_arg);
  }

  if (!textus::base::testing && arg.substr(arg.length() - 8) == "_testing") {
    return 0;
  }

  map<string, initializer *>::iterator map_iterator;
  map_iterator = argument_map().find(arg.c_str());

  if (textus::base::testing && map_iterator == argument_map().end() &&
      arg.substr(arg.length() - 8) == "_testing") {
    arg = arg.substr(0, arg.length()-8);
    map_iterator = argument_map().find(arg.c_str());
  }

  if (map_iterator == argument_map().end()) {
    if (rest_arg != "") {
      return 1;
    }

    if (arg.substr(0,3) != "no-") {
      return 1;
    }

    arg = arg.substr(3);
    map_iterator = argument_map().find(arg.c_str());
    if (map_iterator == argument_map().end() || 
	(*map_iterator).second->func != converter_Bool) {
      return 1;
    }
    rest_arg = "false";
  }

  if (rest_arg == "" && (*map_iterator).second->func == converter_Bool) {
    rest_arg = "true";
  }
    
  if ((*map_iterator).second->func(rest_arg, (*map_iterator).second->arg)) {
    return 1;
  }

  return 0;
}


int ArgumentAppender::processArguments(list<string *> *args)
{
  while (!args->empty()) {
    string full_arg = *(args->front());

    if (full_arg[0] != '-' || full_arg[1] != '-') {
      // it's not an argument, we are done.
      return 0;
    }

    delete args->front();
    args->pop_front();

    if (full_arg.size() == 2) {
      // it's --, we are done.
      return 0;
    }

    string arg;
    string rest_arg;
    string::size_type equals = full_arg.find('=');
    if (equals == string::npos) {
      arg = full_arg.substr(2);
      rest_arg = "";
    } else {
      arg = full_arg.substr(2, equals-2);
      rest_arg = full_arg.substr(equals + 1);
    }

    if (processOneArgument(arg, rest_arg)) {
      usage();
      return 1;
    }

  }
  return 0;
}

int converter_int(const string &s, void *v)
{
  long val;
  char *end;
  int *ip = static_cast<int *>(v);
  val = strtol(s.c_str(), &end, 10);
  if (val > INT_MAX || val < INT_MIN) {
    return -1;
  }

  if (end != s.c_str() + s.length()) {
    return -1;
  }

  *ip = val;
  return 0;
}

int converter_unsigned(const string &s, void *v)
{
  long val;
  char *end;
  unsigned *ip = static_cast<unsigned *>(v);
  val = strtol(s.c_str(), &end, 10);
  if (val > UINT_MAX || val < 0) {
    return -1;
  }

  if (end != s.c_str() + s.length()) {
    return -1;
  }

  *ip = val;
  return 0;
}

int converter_int64_t(const string &s, void *v)
{
  long long val;
  char *end;
  int64_t *lp = static_cast<int64_t *>(v);
  val = strtoll(s.c_str(), &end, 10);
  if (val >= LLONG_MAX || val <= LLONG_MIN) {
    return -1;
  }

  if (end != s.c_str() + s.length()) {
    return -1;
  }

  *lp = val;
  return 0;
}

/*
 * convert_protobuf takes a string from the command line and 
 * makes it into a protobuf.  
 * The following convetions are used to figure out the format:
 *
 * If the first character is in [0-9a-zA-Z./], then it's assumed to be a file(url) name.
 * a + means the protobuf is encoded in the string in base64.
 * a - means the protobuf is encoded in the string in hex.
 * everything else is currently reserved.
 */
int convert_protobuf(const string &s, void *v) {
  string *sp = static_cast<string *>(v);
  char c = s[0];
  if ((c >= '0' && c <= '9') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') ||
      c == '.' || c == '/') {
    AUTODEREF(textus::file::TextusFile *, tf);
    tf = textus::file::TextusFile::openFile(s.substr(1), O_RDONLY);
    if (tf == NULL) {
      return -1;
    }
    *sp = tf->read(MAX_PROTOBUF_ARG_SIZE);
    tf->close();
    return 0;
  }

  if (c == '+') {
    *sp = textus::util::base64_decode(s.substr(1));
    return 0;
  }

  if (c == '-') {
    *sp = textus::util::hex_decode(s.substr(1));
    return 0;
  }

  return -1;
  
}

int converter_string(const string &s, void *v)
{
  string *sp = static_cast<string *>(v);
  *sp = s;
  return 0;
}

int converter_Bool(const string &s, void *v)
{
  using namespace textus::base::logic;
  Bool *bp = static_cast<Bool *>(v);
  if (s == "t" || s == "T" || s == "y" || s == "Y" ||
      s == "True" || s == "true" || s == "TRUE" ||
      s == "Yes" || s == "yes" || s == "YES" ||
      s == "1") {
    *bp = true;
    return 0;
  } else if (s == "F" || s == "f" || s == "n" || s == "N" ||
	     s == "False" || s == "false" || s == "FALSE" ||
	     s == "No" || s == "no" || s == "NO" ||
	     s == "0") {
    *bp = false;
    return 0;
  }

  return -1;
}

int converter_double(const string &s, void *v)
{
  long val;
  char *end;
  double *dp = static_cast<double *>(v);
  val = strtod(s.c_str(), &end);

  if (end != s.c_str() + s.length()) {
    return -1;
  }

  *dp = val;
  return 0;
}



}}};
