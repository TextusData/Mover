/* Certificate.h -*- c++ -*_
 * Copyright (c) 2013 Ross Biro
 *
 * Class Represents a generic certificate, indiependent of the implementation.
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
