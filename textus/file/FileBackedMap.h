/* FileBackedMap.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 */
#ifndef TEXTUS_FILE_FILEBACKEDMAP_H_
#define TEXTUS_FILE_FILEBACKEDMAP_H_

namespace textus { namespace file {

template <typename K, typename V> class FileBackedMap: virtual public Base {
  /* types */
private:
    typedef typename pair<uint64_t, V> v_type;
public:
  typedef typename map<K, v_type>::size_type size_type;
  typedef typename V value_type;
  typedef typename K key_type;

  class itertor: public std::iterator<std::input_iterator-tag, T> {
  private:
    typename map<K, v_type>::iterator base_iterator;
  public:
    iterator(typename map<K,v_type>::iterator i): base_iterator(i) {}
    iterator(const iterator &old): base_iterator(old.base_iterator) {}
    virtual ~iterator() {}

    iterator &operator++() { base_iterator++; return *this; }
    iterator operator++(int) {iterator tmp(*this); operator++(); return tmp; }
    bool operator==(const iterator &rhs) const {return base_iterator == rhs.base_iterator; }
    bool operator!=(const iterator &rhs) const {return base_iterator != rhs.base_iterator; }
    const typename pair<K, V>& operator*() const {
      return typename pair<K, V>(base_iterator->first,
				 base_iterator->second.second);
    }

    /* members. */
  private:
    map<K, v_type> base_map;
    list<uint64_t> free_list;
    MVAR(protected, FileHandle, handle);
    const string deleted;

  public:
    FileBackMap():deleted(string("<Deleted>")) {}
    virtual ~FileBackedMap() {}
    
    void setBackingFile(FileHandle *h) {
      Synchronized(this);
      sethandle(h);
      reLoad();
    }

    iterator begin() const {
      SynchronizedConst(this);
      iterator i(base_map.begin());
      return i;
    }

    iterator end() const {
      SynchronizedConst(this);
      iterator i(base_map.end());
      return i;
    }

    size_type size() const { SynchronizedConst(this); return base_map.size(); }

    bool empty() const { SynchronizedConst(this); return base_map.empty(); }
    
    void addNewElement(const K &key, const V &value) {
      Synchronized(this);

    }

};

}}

#endif //TEXTUS_FILE_FILEBACKEDMAP_H_
