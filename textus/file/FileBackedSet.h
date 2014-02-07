/* FileBackedSet.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Create a set that is backed by a file.
 * must be able to use >> and << operators
 * on the type.
 *
 * Holds everything in memory for now.
 * might change in the future, so try to
 * avoid loading the whole set in.
 */

#ifndef TEXTUS_FILE_FILE_BACKED_SET_H_
#define TEXTUS_FILE_FILE_BACKED_SET_H_

#include "textus/base/Base.h"
#include "textus/file/FileHandle.h"

#include <set>

#define MAX_READ_BUFFER 16384

namespace textus { namespace file {

template <typename T> class FileBackedSet: virtual public Base {
public:
  class iterator: public std::iterator<std::input_iterator_tag, T> {
  private:
    typename map<T, uint64_t>::iterator base_iterator;
  public:
    iterator(typename map<T, uint64_t>::iterator i): base_iterator(i) {}
    iterator(const iterator &old): base_iterator(old.base_iterator) {}
    virtual ~iterator() {}

    iterator& operator++() {base_iterator++;return *this;}
    iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
    bool operator==(const iterator& rhs) const {return base_iterator==rhs.base_iterator;}
    bool operator!=(const iterator& rhs) const {return base_iterator!=rhs.base_iterator;}
    const T& operator*() const {return base_iterator->first;}
  };

  class reverse_iterator: public std::reverse_iterator<T> {
  private:
    typename map<T, uint64_t>::reverse_iterator base_iterator;
  public:
    reverse_iterator(typename map<T, uint64_t>::reverse_iterator i): base_iterator(i) {}
    reverse_iterator(const reverse_iterator &old): base_iterator(old.base_iterator) {}
    virtual ~reverse_iterator() {}

    reverse_iterator& operator++() {base_iterator++;return *this;}
    reverse_iterator operator++(int) {reverse_iterator tmp(*this); operator++(); return tmp;}
    bool operator==(const reverse_iterator& rhs) {return base_iterator==rhs.base_iterator;}
    bool operator!=(const reverse_iterator& rhs) {return base_iterator!=rhs.base_iterator;}
    T& operator*() {return base_iterator->first;}
  };

  typedef typename map<T, uint64_t>::size_type size_type;
  typedef typename map<T, uint64_t>::key_type value_type;

 private:
  map<T, uint64_t> base_set;
  list<uint64_t> free_list;
  MVAR(protected, FileHandle, handle);
  const string deleted;

 public:
  FileBackedSet():deleted(string("<Deleted>")) {}
  virtual ~FileBackedSet() {}

  void setBackingFile(FileHandle *h) {
    Synchronized(this);
    sethandle(h);
    reLoad();
  }

  iterator begin() {
    Synchronized(this);
    iterator i(base_set.begin());
    return i;
  }

  iterator end() {
    Synchronized(this);
    iterator i(base_set.end());
    return i;
  }

  reverse_iterator rbegin() {
    Synchronized(this);
    iterator i(base_set.rbegin());
    return i;
  }

  reverse_iterator rend() {
    iterator i(base_set.rend());
    return i;
  }

  set<T> curreentSet() {
    Synchronized(this);
    return base_set;
  }

  iterator begin() const { 
    SynchronizedConst(this);
    return base_set.begin();
  }

  iterator end() const {
    SynchronizedConst(this);
    return base_set.end(); 
  }

  reverse_iterator rbegin() const {
    SynchronizedConst(this);
    return base_set.rbegin();
  }

  reverse_iterator rend() const { return base_set.rend(); }
  size_type size() const  { SynchronizedConst(this); return base_set.size(); }
  size_type size()  { Synchronized(this); return base_set.size(); }
  size_type max_size() const  { SynchronizedConst(this); return  base_set.max_size(); }
  bool empty() const  { SynchronizedConst(this); return  base_set.empty(); }

  //void swap(Filebackedset &s2) { base_set.swap(s2.base_set); }

  iterator find(const value_type& k) { return base_set.find(k); }
  size_type count(const value_type& k) const { SynchronizedConst(this); return base_set.count(k); }
  iterator lower_bound(const value_type& k) { return base_set.lower_bound(k); }
  iterator upper_bound(const value_type& k) { return base_set.upper_bound(k); }

  pair<iterator, iterator> equal_range(const value_type& k) const { return base_set.equal_range(k); }

  friend bool operator==(const FileBackedSet& s1, const FileBackedSet& s2) { return s1.base_set == s2.base_set; }

  friend bool operator<(const FileBackedSet& s1,const FileBackedSet& s2) { return s1.base_set < s2.base_set; }

  COMPARISON_OPERATORS(FileBackedSet)

  operator set<T>() const {
    return asSet();
  }

  set<T> asSet() const {
    set<T> s;
    SynchronizedConst(this);
    for (typename map<T, uint64_t>::const_iterator i = base_set.begin(); i != base_set.end(); ++i) {
      s.insert(i->first);
    }
    return s;
  }

  pair<iterator, bool> insert(const value_type& x)
  {
    Synchronized(this);
    pair<iterator, bool> p = base_set.insert(pair<value_type, uint64_t>(x, (uint64_t)-1));
    if (p.second) {
      //x->ref();
      addNewElement(x);
    }
    return p;
  }

  void addNewElement(const T &x) {
    Synchronized(this);
    if (free_list.size() > 0) {
      base_set[x] = *free_list.begin();
      free_list.pop_front();
    } else {
      base_set[x] = handle()->seek(0, SEEK_END);
    }
    handle()->seek(base_set[x], SEEK_SET);
    (*handle())  << x+string("\n");
  }

  void removeElement(const T &x) {
    Synchronized(this);
    handle()->seek(base_set[x], SEEK_SET);
    free_list.push_back(base_set[x]);
    *handle() << deleted;
    handle()->seek(0, SEEK_END);
  }

  void processLine(string line, uint64_t offset) {
    static const size_t deleted_length = deleted.length();
    if (line.length() == 0) {
      return;
    }
    if (line.substr(0, deleted_length) == deleted) {
      free_list.push_back(offset);
    } else {
      if (line[line.length() - 1] == '\n') {
	line = line.substr(0, -1);
      }
      base_set[line] = offset;
    }
  }

  void rewrite() {
    Synchronized(this);
    handle()->seek(0, SEEK_SET);
    handle()->truncate();
    for (typename map<T, uint64_t>::const_iterator i = base_set.begin();
	 i != base_set.end();
	 ++i) {
      i->second = handle()->tell();
      *handle() << i->first+string("\n"); 
    }
  }

  void reLoad() {
    string line;
    Synchronized(this);
    base_set.clear();
    free_list.clear();
    for (TextusFile::iterator i = handle()->begin();
	 i != handle()->end();
	 ++i) {
      processLine(*i, i.offset());
    }
  }

  iterator insert(iterator pos, const value_type& x)
  {
    Synchronized(this);
    size_type old_size = base_set.size();
    iterator i = base_set.insert(pos, x);
    if (old_size != base_set.size()) {
      //x->ref();
      addNewElement(x);
    }
    return i;
  }

  template <class InputIterator> void insert(InputIterator first, InputIterator last)
  {
    // XXXXX FIXME: Needs a compile time assert that InputIterator is an InputIterator.
    Synchronized(this);

    for (; first != last; ++first) {
      insert(*first);
    }
    return first;
  }


  void erase(iterator pos)
  {
    Synchronized(this);
    erase(*pos);
    //(*pos)->deref();
  }

  size_type erase(const value_type& k)
  {
    Synchronized(this);
    size_type erased = base_set.erase(k);
    for (size_type s = erased; s > 0; s--) {
      removeElement(k);
      //k->deref();
    }
    return erased;
  }

  void erase(iterator first, iterator last)
  {
    Synchronized(this);
    for (;first != last; ++first) {
      erase(first);
    }
  }

  void clear() {
    Synchronized(this);
    erase(base_set.begin(), base_set.end());
  }
  
};

}} // namespace

#endif // TEXTUS_FILE_FILE_BACKED_SET_H_
