// This file is part of babudb/cpp
//
// Copyright (c) 2010, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "log/log_storage.h"
#include <platform/memory_mapped_file.h>
#include <platform/assert.h>

namespace babudb {

VolatileLogStorage::VolatileLogStorage(Buffer initial_data)
    : keep_data(false) {
  Resize(initial_data.size);
  memcpy(start, initial_data.data, initial_data.size);
}

VolatileLogStorage::VolatileLogStorage(size_t initial_size)
    : keep_data(false) {
  Resize(initial_size);
}

VolatileLogStorage::~VolatileLogStorage() {
  if (!keep_data)
    delete [] start;
}

void VolatileLogStorage::Resize(size_t new_size) {
 char* old_region = start;
 start = new char[new_size];
 if (old_region) {
   memcpy(start, old_region, size);
 }
 if (new_size > size) {
   memset(start + size, 0, new_size - size);
 }
 size = new_size;
 delete [] old_region;
}

PersistentLogStorage* PersistentLogStorage::Open(const string& name) {
  yield::MemoryMappedFile* mfile =
      new YIELD::MemoryMappedFile(name, 1024*1024, O_CREAT|O_RDWR|O_SYNC);
  if (mfile->isOpen()) {
    return new PersistentLogStorage(mfile);
  } else {
    delete mfile;
    return NULL;
  }
}

PersistentLogStorage* PersistentLogStorage::OpenReadOnly(const string& name) {
  yield::MemoryMappedFile* mfile =
      new YIELD::MemoryMappedFile(name, 4, O_RDONLY);
  if (mfile->isOpen()) {
    return new PersistentLogStorage(mfile);
  } else {
    delete mfile;
    return NULL;
  }
}

PersistentLogStorage::PersistentLogStorage(
    yield::MemoryMappedFile* mfile) : memory(mfile) {
  start = memory->getRegionStart();
  size = memory->getRegionSize();
}

void PersistentLogStorage::Resize(size_t new_size) {
  memory->resize(new_size);
  start = memory->getRegionStart();
  size = memory->getRegionSize();
}

bool PersistentLogStorage::Close() {
  return memory->close();
}

bool PersistentLogStorage::IsWritable() {
	return (memory->getFlags() & O_RDWR) != 0;
}

void PersistentLogStorage::WriteBack() {
  memory->writeBack();
}

void PersistentLogStorage::WriteBack(void* ptr, size_t length) {
  memory->writeBack(ptr, length);
}

};  // namespace babudb
