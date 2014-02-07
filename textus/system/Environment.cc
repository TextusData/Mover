/* Environmanet.cc -*- C++ -*-
 * Copyright (C) 2010-2013 Ross Biro
 *
 * A class process environment variables.
 */

#include "textus/system/Environment.h"
#include "textus/base/AutoDeref.h"

namespace textus { namespace system {

static textus::base::Base SyncObj;

Environment &Environment::mutableSystemEnvironment() {
  // XXXXX Fixme: never deallocated.
  static textus::base::AutoDeref<Environment> static_env(new Environment(textus::base::init::initEnvironment()));;
  return *static_env;
}

const Environment &Environment::systemEnvironment() {
  return mutableSystemEnvironment();
}

void Environment::modifySystemEnvironment(string key, string value) {
  mutableSystemEnvironment().setVariable(key, value);
}

int Environment::exportSystemEnvironment(string key, string value) {
  int ret;
  modifySystemEnvironment(key, value);
  HRC(setenv(key.c_str(), value.c_str(), 1));

 error_out:
  return ret;
}

string Environment::getConfigPath(string file) {
  string r = systemEnvironment().getVariable("APP_PATH")
    + "/" + file;
  return r;
}

string Environment::getTmpPath(string file) {
  string r = systemEnvironment().getVariable("APP_PATH")
    + "/" + file;
  return r;
}

}} //namespace
