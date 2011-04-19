// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, 2010 Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "index.h"
#include "util.h"

#include "index_writer.h"
#include "babudb/log/sequential_file.h"

#include "yield/platform/assert.h"
#include "yield/platform/directory_walker.h"
#include "yield/platform/disk_operations.h"

#include <sstream>
#include <algorithm>

#define POSIX  // for O_ definitions from fcntl.h
#include <fcntl.h>

using namespace std;
using namespace babudb;

ImmutableIndex::ImmutableIndex(LogStorage* mm,
                               const KeyOrder& order, lsn_t lsn)
    : storage(mm), index(MapCompare(order)), order(order), latest_lsn(lsn)  {}

ImmutableIndex* ImmutableIndex::Load(const string& name, lsn_t lsn,
                                     const KeyOrder& order) {
  LogStorage* mmap = PersistentLogStorage::OpenReadOnly(name);
  if (mmap == NULL)
     return NULL;

  ImmutableIndex* result = new ImmutableIndex(mmap, order, lsn);
  if (!result->LoadRoot()) {
    delete result;
    return false;
  }
  return result;
}

string ImmutableIndex::GetIndexName(const string& name, lsn_t lsn) {
  std::ostringstream file_name;
  file_name << name << "_" << lsn << ".idx";
  return file_name.str();
}

ImmutableIndexWriter* ImmutableIndex::Create(
    const string& name, lsn_t lsn, size_t chunk_size) {
  LogStorage* mfile = PersistentLogStorage::Open(GetIndexName(name, lsn));
  if (!mfile) {
    return NULL;
  }
  ImmutableIndexWriter* result = new ImmutableIndexWriter(mfile, chunk_size);
  return result;
}

ImmutableIndex::Tree::iterator ImmutableIndex::findChunk(const Buffer& key) {
  if(index.size() == 0)
    return index.end();

  Tree::iterator cursor = index.lower_bound(key);
  // cursor is equal or larger than key

  if(cursor == index.end()) {
    --cursor; // we might still be in the last partition
  }
  else if(order.less(key,cursor->first)) {
    // cursor is larger than key

    if(cursor != index.begin())
      --cursor;
  }

  return cursor;
}

offset_t* ImmutableIndex::getOffsetTable(offset_t offset_rec_offset) {
  SequentialFile::Record* offsets_rec = storage.offset2record(offset_rec_offset);
  ASSERT_TRUE(offsets_rec->getType() == RECORD_TYPE_OFFSETS);
  offset_t* value_offsets = (offset_t*)offsets_rec->getPayload();
  return value_offsets;
}

Buffer ImmutableIndex::Lookup(Buffer search_key) {
  ImmutableIndexIterator i = Find(search_key);

  if(i != end()) { // i >= search_key
    if(!order.less(search_key, (*i).first)) // ==
      return (*i).second;
    else
      return Buffer::Empty();
  }
  else
    return Buffer::Empty();
}

ImmutableIndex::iterator ImmutableIndex::Find(Buffer search_key) {
  Tree::iterator cursor = findChunk(search_key);

  if(cursor == index.end()) {
    return end(); // not found
  }
  offset_t* value_offsets = getOffsetTable(cursor->second);

  SequentialFile::iterator file_cursor = storage.at(cursor->second);
  int record_count = 0;
  while (file_cursor.GetNext() && file_cursor.IsType(RECORD_TYPE_KEY)) {
    Buffer key(file_cursor.GetRecord()->getPayload(), file_cursor.GetRecord()->getPayloadSize());

    if(!order.less(key,search_key)) { 		// found key >= search_key
      return ImmutableIndex::iterator(storage,value_offsets,file_cursor,record_count);
    }
    record_count++;
  }

  return end(); // not found
}

ImmutableIndexIterator ImmutableIndex::begin() const {
  return ImmutableIndexIterator(storage,false);
}

ImmutableIndexIterator ImmutableIndex::end() const {
  return ImmutableIndexIterator(storage,true);
}

bool ImmutableIndex::LoadRoot() {
  SequentialFile::iterator cursor = storage.Last();
  ASSERT_TRUE(cursor.GetPrevious() != NULL);

  if(cursor.GetRecord()->getType() != RECORD_TYPE_FILE_FOOTER)
    return false;

  while (cursor.GetPrevious()) {  // skip footer and seek to end of section 
    if (cursor.GetRecord()->getType() == RECORD_TYPE_INDEX_OFFSETS) {
      break;
    }
  }

  // If they index file is not empty...
  if (cursor.GetRecord()->getPayloadSize() > 0) {
    offset_t* offsets = (offset_t*)cursor.GetRecord()->getPayload();
    size_t no_offsets = cursor.GetRecord()->getPayloadSize()/sizeof(offset_t);

    size_t record_count = 0;
    while (cursor.GetNext()) {	
      if(cursor.GetRecord()->getType() != RECORD_TYPE_FILE_FOOTER) {
        ASSERT_TRUE(cursor.GetRecord()->getType() == RECORD_TYPE_INDEX_KEY);
        Buffer key(cursor.GetRecord()->getPayload(),
             cursor.GetRecord()->getPayloadSize());

        ASSERT_TRUE(record_count < no_offsets);

        index.insert(pair<Buffer,offset_t>(key, offsets[record_count]));
        record_count++;
      }
    }
    ASSERT_TRUE(record_count > 0);
  }

  return true;
}

ImmutableIndex::DiskIndices ImmutableIndex::FindIndices(const string& name_prefix) {
  DiskIndices results;

  pair<YIELD::Path,YIELD::Path> dir_prefix = YIELD::Path(name_prefix).split();
  YIELD::DirectoryWalker walker(dir_prefix.first);

  while(walker.hasNext()) {
    auto_ptr<YIELD::DirectoryEntry> entry = walker.getNext();

    lsn_t lsn;
    if(matchFilename(entry->getPath(), dir_prefix.second.getHostCharsetPath(), "idx", lsn)) {
      results.push_back(make_pair(entry->getPath(), lsn));
    }
  }

  return results;
}

static bool MoreRecent(const std::pair<YIELD::Path,lsn_t>& one,
                       const std::pair<YIELD::Path,lsn_t> two) {
  return one.second > two.second;
}

ImmutableIndex* ImmutableIndex::LoadLatestIntactIndex(
    DiskIndices& on_disk, const KeyOrder& order) {
  // find latest intact ImmutableIndex
  YIELD::Path latest_intact_path("");
  sort(on_disk.begin(), on_disk.end(), MoreRecent);

  for (DiskIndices::iterator i = on_disk.begin(); i != on_disk.end(); ++i) {
    ImmutableIndex* result = Load(i->first, i->second, order);
    if (result) {
      return result;
    }
  }
  return NULL;
}

void ImmutableIndex::CleanupObsolete(const string& file_name, const string& to) {
  DiskIndices on_disk = FindIndices(file_name);
  lsn_t current_lsn = GetLastLSN();

  for (DiskIndices::iterator i = on_disk.begin(); i != on_disk.end(); ++i) {
    if (i->second < current_lsn) {  // not the latest intact index
      pair<YIELD::Path,YIELD::Path> parts = i->first.split();
      YIELD::DiskOperations::rename(i->first, to + parts.second.getHostCharsetPath());
    }
  }
}

int ImmutableIndex::Read(int offset, char* buffer, int bytes) {
  int remaining_bytes = min(bytes, (int)storage.GetLogStorage()->Size() - offset);
  memcpy(buffer, storage.GetLogStorage()->Start(), remaining_bytes);
  return remaining_bytes;
}
