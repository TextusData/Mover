/* Members.h -*- C++ -*-
 * Copyright (C) 2010-2014 Ross Biro
 *
 * A set of macros to define member variables and appropriate accessors.
 */
// Only works in private.

#ifndef TEXTUS_BASE_MEMBERS_H_
#define TEXTUS_BASE_MEMBERS_H_

namespace texteus { namespace base {

// macros don't have namespaces, but we might add some code here eventually.


#define MSTRING(prot, name)						\
  private:								\
  std::string name##_;							\
prot:									\
 std::string name() { Synchronized(this); return name##_; }		\
 const std::string name() const { SynchronizedConst(this); return name##_; } \
 void set##name(std::string s) { Synchronized(this); name##_ = s; }	\
private:

#define MINT(prot, name)						\
  private:								\
  int name##_;								\
prot:									\
 int name() { Synchronized(this); return name##_; }			\
 int name() const { SynchronizedConst(this); return name##_; }		\
 void set##name(int s) { Synchronized(this); name##_ = s; }		\
private:
 

#define MVAR(prot, type, name)			\
private:					\
 textus::base::AutoDeref<type> name##_;		\
prot :						\
 type *name() { Synchronized(this); return name##_; }	\
 const type *name() const { SynchronizedConst(this); return name##_; }	\
 void set##name(type *n) { Synchronized(this); name##_ = n; }	\
 void set_##name(type *n) { Synchronized(this); name##_ = n; }	\
private:

// Only works in private.
#define CMVAR(prot, type, name)			\
private:					\
 textus::base::AutoDeref<const type> name##_;		\
prot :						\
 const type *name() const { SynchronizedConst(this); return name##_; }	\
 void set##name(const type *n) { SynchronizedConst(this); name##_ = n; }	\
private:

// Only works in private.
#define WMVAR(prot, type, name)			\
private:					\
 textus::base::AutoWeakDeref<type> name##_;		\
prot :						\
 type *name() { Synchronized(this); return name##_; }	\
 const type *name() const { SynchronizedConst(this); return name##_; }  \
 void set##name(type *n) { Synchronized(this); name##_ = n; }	\
private:

// Only works in private.
#define WCMVAR(prot, type, name)			\
private:					\
 textus::base::AutoWeakDeref<const type> name##_;		\
prot :						\
 const type *name() const { SynchronizedConst(this); return name##_; }	\
 void set##name(const type *n) { SynchronizedConst(this); name##_ = n; }	\
private:

}} //namespace



#endif //TEXTUS_BASE_MEMBERS_H_