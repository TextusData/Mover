/* UUID.h -*- C++ -*-
 * Copyright (C) 2010, 2013 Ross Biro
 *
 * A class to represent a UUID.
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

#ifndef TEXTUS_BASE_UUID_H_
#define TEXTUS_BASE_UUID_H_

#include <textus/base/Base.h>
#include <textus/base/Utility.h>

#include <uuid/uuid.h>
#include <string.h>
#include <memory.h>

#include <iostream>
#include <string>

namespace textus { namespace base {
using namespace std;

class UUID /* Lightweight class, no base. */ {
private:
  // uuid_t is an array, so it makes sense to pass it by value and
  // have it modified.
  uuid_t uuid;

public:
  UUID() {}

  explicit UUID(uuid_t u) {
    memcpy (uuid, u, sizeof (uuid_t));
  }

  UUID(const UUID &u) {
    memcpy (uuid, u.uuid, sizeof (uuid_t));
  }

  explicit UUID(string s) {
    uuid_parse(s.c_str(), uuid);
  }

  explicit UUID(class UUIDPB);

  ~UUID() {}
  
  bool operator ==(const UUID &u2) const {
    return uuid_compare(uuid, u2.uuid) == 0;
  }

  bool operator <(const UUID &u2) const {
    return uuid_compare(uuid, u2.uuid) < 0;
  }

  COMPARISON_OPERATORS(UUID);

  std::string toString() const {
    char buff[40]; // string size is 36+nul.  Give us some extra space.
    uuid_unparse_lower(uuid, buff);
    return string(buff);
  }

  friend std::ostream& operator << (std::ostream& o, const UUID& u);
  friend std::ostream& operator << (std::ostream& o, const UUID * const u);

  void bits(unsigned char b[16]) {
    memcpy (b, uuid, sizeof(b));
  }

  void generate() {
#ifdef HAVE_UUID_GENERATE_TIME_SAFE
    if (uuid_generate_time_safe(uuid) < 0) {
      LOG(WARNING) << "uuid_generate_time_safe was unsafe.  Falling Back to uuid_generate.  May lose time information.\n";
      uuid_gerneate(uuid);
    }
#else
  uuid_generate_time(uuid);
#endif
  }

};

}} // namespace

#endif // TEXTUS_BASE_UUID_H_
