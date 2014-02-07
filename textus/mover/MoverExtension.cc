/* MoverExtension.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 */
#include "textus/mover/Mover.h"
#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/MoverExtension.h"

namespace textus { namespace mover {

ReferenceValueMap<UUID, MoverExtension *> MoverExtension::extensions;


}} //namespace
