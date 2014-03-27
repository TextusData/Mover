/* KeyBook.h -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
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

#ifndef TEXTUS_MOVER_KEYBOOK_H_
#define TEXTUS_MOVER_KEYBOOK_H_

#include <string>

#include "textus/base/Base.h"
#include "textus/base/ReferenceList.h"
#include "textus/mover/MultiKeyDescription.h"
#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/Mover.h"


namespace textus { namespace mover {


#define HUMAN_READABLE_NAME std::string("_human_readable_name")

using namespace textus::base;
using namespace std;


class KeyBook: virtual public textus::base::Base {
private:
  ReferenceList<MultiKeyDescription *> descriptions;

protected:
public:
  explicit KeyBook() {}
  virtual ~KeyBook() {}

  int readFileSearchPath(const list<string> &paths, string name);
  bool process(Mover *mover, string hash);
  bool processData(string data);

};

}} //namespace

#endif //TEXTUS_MOVER_KEYBOOK_H_
