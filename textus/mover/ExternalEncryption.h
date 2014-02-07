/* ExternalEncryption.h -*- c++ -*-
 * Copyright (C) 2013, 2014 Ross Biro
 *
 */
#ifndef TEXTUS_MOVER_EXTERNALENCRYPTION_H_
#define TEXTUS_MOVER_EXTERNALENCRYPTION_H_

#include "textus/mover/MoverEncryption.h"
#include "textus/config/Config.h"

namespace textus { namespace mover {

class ExternalEncryption: public MoverEncryption {
private:
  string encrypt_template;
  string decrypt_template;
  string new_key_template;
  string export_public_template;
  string import_public_template;
  string delete_public_template;
  string delete_private_template;
  string get_key_name_template;
  string key_name_group_no;
  string key_name;
  ReferenceValueMap<string, textus::config::ConfigData *> *key_desc;

protected:
  int processTemplate(string in, string *out, KeyDescription *kd);
  int getEncryptCommand(string *out,  KeyDescription *kd);
  int getDecryptCommand(string *out, KeyDescription *kd);
  int getGenKeyCommand(string *out, KeyDescription *kd);
  int getDeletePrivateCommand(string *out, KeyDescription *kd);
  int getDeletePublicCommand(string *out, KeyDescription *kd);
  int getKeyNameGroupNo(string *out, KeyDescription *kd);
  int getKeyNameRE(string *out, KeyDescription *kd);

  ReferenceValueMap<string, textus::config::ConfigData *> *keyDesc() {
    return key_desc;
  }

  string getDefault(textus::config::ConfigData *cd);

public:
  static ExternalEncryption *fromConfig(string name, 
					textus::config::ConfigData *r);

  explicit ExternalEncryption(): key_name("key-name"), key_desc(NULL) {}
  virtual ~ExternalEncryption() { if (key_desc != NULL) key_desc->deref(); }

  virtual int newKey(KeyDescription *kd);
  virtual KeyDescription *key(KeyDescription *in=NULL);
  virtual int deleteKey(KeyDescription *kd);
  virtual int encrypt(KeyDescription *, string in, string *out);
  virtual int decrypt(KeyDescription *, string in, string *out, string *key_name);
  virtual set<string> getDecryptionKeywords();
  virtual string keyName() const {
    return key_name;
  }
  
  
};

DECLARE_STRING_ARG(mover_template_backing_file);

}} //namespace

#endif //TEXTUS_MOVER_EXTERNALENCRYPTION_H_
