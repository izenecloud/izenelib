// This file is part of babudb/cpp
//
// Copyright (c) 2010, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// Abstract from logs in files and in memory

#ifndef BABUDB_LOGSTORAGE_H
#define BABUDB_LOGSTORAGE_H

#include "babudb/buffer.h"

#include <stddef.h>
#include <memory>
using std::auto_ptr;
#include <string>
using std::string;

namespace yield {
class MemoryMappedFile;
};

namespace babudb {

class LogStorage {
 public:
  LogStorage() : size(0), start(NULL) {}
  virtual ~LogStorage() {}

	inline char* Start() { return start; }
	inline char* End() { return start + size; }
  inline size_t Size() { return size; }

	virtual void WriteBack() = 0;
	virtual void WriteBack(void* ptr, size_t length) = 0;

  virtual void Resize(size_t new_size) = 0;
	virtual bool Close() = 0;
  virtual bool IsWritable() = 0;

 protected:
	size_t size;
	char* start;
};

class VolatileLogStorage : public LogStorage {
 public:
  VolatileLogStorage(Buffer initial_data);
  VolatileLogStorage(size_t initial_size);
  ~VolatileLogStorage();

  virtual void WriteBack() {}
  virtual void WriteBack(void* ptr, size_t length) {}

  virtual void Resize(size_t new_size);
  virtual bool Close() { return true; }
  virtual bool IsWritable() { return false; }

  // Do not delete underlying memory at destruction time
  void KeepData() { keep_data = true; }
protected:
  bool keep_data;
};

class PersistentLogStorage : public LogStorage {
 public:
  static PersistentLogStorage* Open(const string& name);
  static PersistentLogStorage* OpenReadOnly(const string& name);

	virtual void WriteBack();
	virtual void WriteBack(void* ptr, size_t length);

  virtual void Resize(size_t new_size );
	virtual bool Close();
  virtual bool IsWritable();
protected:
  PersistentLogStorage(yield::MemoryMappedFile*);
	auto_ptr<yield::MemoryMappedFile> memory;
};

}  // namespace babudb

#endif  // BABUDB_LOGSTORAGE_H
