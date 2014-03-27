/* ExternalEncryption.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
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

#include "textus/mover/ExternalEncryption.h"
#include "textus/config/Config.h"
#include "textus/file/Shell.h"
#include "textus/template/Template.h"
#include "textus/base/init/Init.h"
#include "textus/system/Environment.h"
#include "textus/util/Regexp.h"

namespace textus { namespace mover {

using namespace textus::config;
using namespace textus::file;
using namespace textus::template_;

DEFINE_STRING_ARG(mover_template_backing_file, "mover_key_counter", "mover_count_file", "The backing file we use to make sure we don't generate two idential key names or file names.");


ExternalEncryption *ExternalEncryption::fromConfig(string name, ConfigData *cd)
{
  ExternalEncryption *ee=NULL;
  int ret = 0;
  ee = new ExternalEncryption();
  HRNULL(ee);
  HRC(cd->getMapByName("key-desc", &ee->key_desc));
  HRC(cd->getStringByName("encrypt", &ee->encrypt_template));
  HRC(cd->getStringByName("decrypt", &ee->decrypt_template));
  HRI(cd->getStringByName("genkey", &ee->new_key_template));
  HRI(cd->getStringByName("export-public", &ee->export_public_template));
  HRI(cd->getStringByName("import-public", &ee->import_public_template));
  HRI(cd->getStringByName("delete-public", &ee->delete_public_template));
  HRI(cd->getStringByName("delete-private", &ee->delete_private_template));
  HRI(cd->getStringByName("get-key-name", &ee->get_key_name_template));
  HRI(cd->getStringByName("key-name-group-no", &ee->key_name_group_no));
  HRI(cd->getStringByName("key-name", &ee->key_name));

  return ee;
 error_out:
  if (ee) {
    delete ee;
  }
  return NULL;
}

int ExternalEncryption::processTemplate(string in, string *out,
					KeyDescription *kd) {
  int ret=0;
  kd->setVariables();
  AUTODEREF(Template<KeyDescription *> *, t);
  t = new Template<KeyDescription *>(kd);
  HRNULL(t);
  HRC(t->setBackingFile(mover_template_backing_file));
  LOG(DEBUG) << "processTemplate(" << in << ", ...)\n";
  HRC(t->process(in, out));
 error_out:
  return ret;

}

int ExternalEncryption::getEncryptCommand(string *out, KeyDescription *kd) {
  return processTemplate(encrypt_template, out, kd);
}

int ExternalEncryption::getDeletePrivateCommand(string *out,
						KeyDescription *kd) {
  return processTemplate(delete_private_template, out, kd);
}

int ExternalEncryption::getDeletePublicCommand(string *out,
					       KeyDescription *kd) {
  return processTemplate(delete_public_template, out, kd);
}

int ExternalEncryption::getDecryptCommand(string *out, KeyDescription *kd) {
  return processTemplate(decrypt_template, out,  kd);
}

int ExternalEncryption::getKeyNameGroupNo(string *out, KeyDescription *kd) {
  return processTemplate(key_name_group_no, out,  kd);
}

int ExternalEncryption::getKeyNameRE(string *out, KeyDescription *kd) {
  return processTemplate(get_key_name_template, out,  kd);
}


int ExternalEncryption::getGenKeyCommand(string *out, KeyDescription *kd) {
  return processTemplate(new_key_template, out, kd);
}

set<string> ExternalEncryption::getDecryptionKeywords() {
  int ret = 0;

  AUTODEREF(Template<KeyDescription *> *, t);
  t = new Template<KeyDescription *>(NULL);
  HRNULL(t);


 error_out:
  if (ret == 0) {
    return t->findKeyWords(decrypt_template);
  }

  set<string> keywords;
  keywords.insert("invalid_keyword");
  return keywords;
  
}

int ExternalEncryption::deleteKey(KeyDescription *kd) {
  string command;
  int ret = 0;
  AUTODEREF(Shell *, s);
  HRC(getDeletePrivateCommand(&command, kd));
  if (command != "") {
    s = new Shell();
    HRNULL(s);

    s->setCommand(command);
    s->go();
    s->waitForCompletion();
    HRC(s->exitStatus());
  }

  command = "";
  HRC(getDeletePublicCommand(&command, kd));
  if (command != "") {
    s = new Shell();
    HRNULL(s);

    s->setCommand(command);
    s->go();
    s->waitForCompletion();
    HRC(s->exitStatus());
  }

 error_out:
  return ret;
}

int ExternalEncryption::encrypt(KeyDescription *kd, string data, string *out) {
  string command;
  int ret = 0;
  AUTODEREF(Shell *, s);
  HRC(getEncryptCommand(&command, kd));
  s = new Shell();
  HRNULL(s);

  s->setCommand(command);
  s->setInput(data);
  s->go();
  s->waitForCompletion();
  HRC(s->exitStatus());

  *out = s->getOutput();
 error_out:
  return ret;
}

int ExternalEncryption::decrypt(KeyDescription *kd, string data, string *out,
				string *key_name) {
  int ret = 0;
  string command;
  AUTODEREF(Shell *, s);
  HRC(getDecryptCommand(&command, kd));
  s = new Shell();
  HRNULL(s);
  s->setCommand(command);
  s->setInput(data);
  s->go();
  s->waitForCompletion();
  HRC(s->exitStatus());
  *out = s->getOutput();
  if (key_name) {
    if (get_key_name_template != "") {
      command = "";
      HRC(getKeyNameRE(&command, kd));
      if (command != "") {
	textus::util::RegExp<char> re;
	ReferenceList<textus::util::Match *> matches;
	textus::util::Match *match;
	size_t gn;
	string err;
	LOG(DEBUG) << "compiling /" << command << "/ to find key-name.\n";
	HRC(re.compile(command));
	command = "";
	HRC(getKeyNameGroupNo(&command, kd));
	gn = atoi(command.c_str());
	err = s->getErrorOutput();
	HRTRUE(re.match(err, &matches));
	HRTRUE(matches.size() > 0);
	match = matches.front();
	HRTRUE(gn < match->ngroups);
	off_t start;
	off_t end;
	start = match->group_matches[gn].front().first;
	end = match->group_matches[gn].front().second;
	HRTRUE (start < end);
	*key_name = err.substr(start, end - start);
	LOG(DEBUG)<< "found keyname: " << *key_name << "\n";
      }
    } else {
      *key_name = kd->at(keyName());
    }
  }
 error_out:
  return ret;
}

int ExternalEncryption::newKey(KeyDescription *kd) {
  int ret=0;
  AUTODEREF(Shell *, s);
  string command;
  HRC(getGenKeyCommand(&command, kd));
  s = new Shell();
  HRNULL(s);
  s->setCommand(command);
  s->go();
  s->waitForCompletion();
  HRC(s->exitStatus());
 error_out:
  return ret;
}

string ExternalEncryption::getDefault(ConfigData *cd) {
  bool file = false;
  if (cd->type() != ConfigData::mdata) {
    return "";
  }
  if (cd->asMap().count(string("type")) > 0) {
    ConfigData *c2 = cd->asMap()[string("type")];
    if (c2->type() == ConfigData::sdata) {
      if (c2->asString() == "file") {
	file = true;
      }
    }
  }
  if (cd->asMap().count(string("default")) == 0) {
    return "";
  }
  cd = cd->asMap()[string("default")];
  if (cd ->type() != ConfigData::sdata) {
    return "";
  }

  string s = cd->asString();

  if (file && s.find('/') == string::npos) {
    s = textus::system::Environment::getDataPath(s);
  }

  return s;
}


KeyDescription *ExternalEncryption::key(KeyDescription *in) {
  AUTODEREF(KeyDescription *, holder);
  int ret = 0;
  if (in == NULL) {
    holder = new KeyDescription();
    if (holder == NULL) {
      return NULL;
    }
    in = holder;
  }
 
  in->ref();

  for (ReferenceValueMap<string, ConfigData *>::iterator i = keyDesc()->begin();
       i != keyDesc()->end();
       ++i) {
    if (in->count(i->first) == 0) {
      ConfigData *cd = i->second;
      string s;
      HRC(processTemplate(getDefault(cd), &s, in));
      in->set(i->first, s);
    }
  }
  in->setEncryption(this);
  return in;
 error_out:
  in->deref();
  return NULL;
}

}} //namespace
