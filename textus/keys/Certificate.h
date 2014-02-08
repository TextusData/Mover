/* Certificate.h -*- c++ -*_
 * Copyright (c) 2013 Ross Biro
 *
 * Class Represents a generic certificate, indiependent of the implementation.
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

#ifndef TEXTUS_KEY_CERTIFICATE_H_
#define TEXTUS_KEY_CERTIFICATE_H_

#include "textus/base/Base.h"

namespace textus { namespace keys { 

class Certificate: virtual public Base {
private:
public:
  Certificate() {}
  virtual ~Certificate() {}

  virtual string name()=0; // Human readable, if availabled.
  virtual string setName(string s)=0;

  virtual string id()=0; // computer readable, must be available.
  virtual string setId(string s)=0;

  virtual bool validate(Certificate *cert)=0;

  virtual bool expired()=0;
  virtual bool valid()=0;

  // XXXXX Fxime: is this good enough?
  // mistakes here would be bad in the Ghost Busters sense.
  virtual bool operator==(const Certificate &c1) const {
    return id() == c1.id();
  }

};

}} //namespace

#endif // TEXTUS_KEY_CERTIFICATE_H_
