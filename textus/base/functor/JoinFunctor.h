/* JoinFunctor.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * A functor to do a string join.
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

#ifndef TEXTUS_BASE_FUNCTOR_JOIN_FUNCTOR_H_
#define TEXTUS_BASE_FUNCTOR_JOIN_FUNCTOR_H_

#include "textus/base/BaseString.h"
#include "textus/base/functor/Functor.h"

namespace textus { namespace base { namespace functor {

using namespace textus::base;

class JoinFunctor: public AccumulationFunctor {
private:
  BaseString result;
  string sep;

public:
  explicit JoinFunctor(string seperator=", "): sep(seperator) {}
  virtual ~JoinFunctor() {}
  
  virtual Base *apply(Base *b) {
    BaseString *bs = dynamic_cast<BaseString *>(b);
    if (result.length() == 0) {
      result.append(static_cast<string>(*bs));
    } else {
      result.append(sep + static_cast<string>(*bs));
    }
    return &result;
  }

  virtual Base *finalize() {
    return &result;
  }
};

}}} //namespace

#endif // TEXTUS_BASE_FUNCTOR_JOIN_FUNCTOR_H_
 
