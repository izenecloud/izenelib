// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#ifndef BABUDB_BUFFER_H
#define BABUDB_BUFFER_H

#include <cstring>

#include <string>
using std::string;

namespace babudb {

class Buffer {
public:
  Buffer(const Buffer& data) {
    this->data = data.data;
    this->size = data.size;
  }

  explicit Buffer(void* data, size_t size) {
    this->data = data;
    this->size = size;
  }


  // Buffer::create* functions allocate memory and copy the data
  // must be .free()d later

  static Buffer create(size_t size) {
    Buffer result;
    result.data = (void*)new char[size];
    result.size = size;
    return result;
  }

  static Buffer createFrom(const string& data) {
    return createFrom((void*)data.c_str(),data.size());
  }

  static Buffer createFrom(void* data, size_t size) {
    Buffer result = create(size);
    memcpy(result.data,data,size);
    return result;
  }

  static Buffer createFrom(int data) {
    Buffer result = create(sizeof(int));
    memcpy(result.data,&data,sizeof(int));
    return result;
  }


  // Buffer::wrap only wrap data without allocation and copying

  static Buffer wrap(const int& data) {
    return Buffer((void*)&data,sizeof(int));
  }

  static Buffer wrap(const unsigned int& data) {
    return Buffer((void*)&data,sizeof(unsigned int));
  }

  static Buffer wrap(const long long& data) {
    return Buffer((void*)&data,sizeof(long long));
  }

  static Buffer wrap(const unsigned long long& data) {
    return Buffer((void*)&data,sizeof(unsigned long long));
  }
  
  static Buffer wrap(const char* str, int size) {
    return Buffer((void*)str, size);
  }

  static Buffer wrap(const char* str) {
    return Buffer((void*)str, strlen(str));
  }

  static Buffer wrap(const string& str) {
    return Buffer((void*)str.c_str(),str.size());
  }

  /*
  template <class T>
  static Buffer createFrom(T data) {
    Buffer result = create(sizeof(T));
    memcpy(result.data,&data,sizeof(T));
    return result;
  }
  */
  int getAsInt() {
    return *(int*)data;
  }

  unsigned long long getAsUInt64() {
    return *(unsigned long long*)data;
  }

  // Represents a deleted data item in overlay indices.

  static Buffer Deleted() {
    return Buffer(0,-1);
  }

  bool isDeleted() const {
    return data == 0 && size == -1;
  }
  
  operator bool () const {
    return !isDeleted();
  }

  // An empty value

  static Buffer Empty() {
    return Buffer(0,0);
  }

  bool isEmpty() const {
    return data == 0 && size != -1;
  }


  void* operator * () {
    return data;
  }

  void free() {
    delete [] (char*)data;
    data = 0; size = 0;
  }

  Buffer clone() const {
    if(isEmpty())
      return Empty();
    else if(isDeleted())
      return Deleted();
    else
      return createFrom(data,size);
  }

  void copyTo(void* dest, size_t max_size) const {
    memcpy(dest, data, size);
  }

  bool operator == (const string& s) const {
    return string((char*)this->data,this->size) == s;
  }

  bool operator == (const Buffer& s) const {
    if(this->size != s.size) return false;

    for(int i = 0; i < (int)this->size; ++i )
      if( ((char*)this->data)[i] != ((char*)s.data)[i])
        return false;

    return true;
  }

  void* data;
  int size;

private:
  Buffer() : data(0), size(0) {}
};

class DataHolder {
public:
  DataHolder(const string& str)
    : data(Buffer::createFrom(str)) {}

  ~DataHolder() {
    data.free();
  }

  operator Buffer () const {
    return data;
  }

  operator Buffer& () {
    return data;
  }

  Buffer data;
};

#define MAX_LSN 0xFFFFffff
typedef unsigned int lsn_t;

}

#endif
