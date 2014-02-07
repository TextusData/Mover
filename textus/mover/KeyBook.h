/* KeyBook.h -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
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

};

}} //namespace

#endif //TEXTUS_MOVER_KEYBOOK_H_
