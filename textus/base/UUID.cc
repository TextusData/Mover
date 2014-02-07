/* UUID.h -*- C++ -*-
 * Copyright (C)  2013 Ross Biro
 *
 * A class to represent a UUID.
 */


#include "UUID.h"

namespace textus { namespace base {

std::ostream& operator<< (std::ostream& o, const UUID & uuid)
{
  return o << uuid.toString();
}

std::ostream& operator<< (std::ostream& o, const UUID * const uuid)
{
  return o << uuid->toString();
}

}} //namespace
