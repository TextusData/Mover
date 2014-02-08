/* MoverEncryption.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
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

#ifndef TEXTUS_MOVER_MOVERENCRYPTION_H_
#define TEXTUS_MOVER_MOVERENCRYPTION_H_

#include <string>
#include <map>
#include <set>

#include "textus/base/Base.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/mover/KeyDescription.h"

namespace textus { namespace mover {

using namespace textus::base;
using namespace std;

class MoverEncryption: virtual public Base {
public:
  typedef ReferenceValueMap<string, MoverEncryption *>::iterator iterator;

private:
  static ReferenceValueMap<string, MoverEncryption *> encryptions;

protected:
public:
  static MoverEncryption *findEncryption(string name);
  static int registerEncryption(string name, MoverEncryption *);
  static int fromConfigFile(string name); // put this here so it
					  // forces ExternalEncryption
					  // to get linked in.
  static iterator begin() { return encryptions.begin(); }
  static iterator end() { return encryptions.end(); }

  explicit MoverEncryption() {}
  virtual ~MoverEncryption() {}

  virtual int newKey(KeyDescription *)=0;
  virtual int deleteKey(KeyDescription *) = 0;

  virtual int encrypt(KeyDescription *, string in, string *out)=0;
  virtual int decrypt(KeyDescription *, string in, string *out, string *key = NULL)=0;

  virtual KeyDescription *key(KeyDescription *in=NULL)=0;

  virtual string keyName() const = 0;

  virtual set<string> getDecryptionKeywords() {
    set<string> s;
    s.insert("invalid_keyword");
    return s;
  }

  virtual KeyDescription *tempKey(KeyDescription *in=NULL) {
    AUTODEREF(KeyDescription *, hold);
    if (in == NULL) {
      hold = new KeyDescription();
      in = hold;
    }
    in->setTemporary();
    in->setEncryption(this);
    return key(in);
  }

};

}} //namespace


#endif //TEXTUS_MOVER_MOVERENCRYPTION_H_
