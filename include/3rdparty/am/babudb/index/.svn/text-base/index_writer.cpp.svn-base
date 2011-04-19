// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, 2010 Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "index_writer.h"

#include <yield/platform/assert.h>
#include <yield/platform/memory_mapped_file.h>

#include "log_index.h"
#include "index.h"
#include "babudb/key.h"

using namespace babudb;

/** Persistent immutable index. Contains key-value data pairs.

  Uses the records of the SequentialFile. Three record types:
  - Offsets: array of fixed-length offsets, contains offsets
    for the following key records
  - Key: a key
  - Value: a value

  These records are arranged like:

  [[Values] Offsets [Keys]] ... Offset_Offsets [Key]

  The last set of keys are an index into the file.

  TODO: lookup code might be simpler if we point to the last key
      instead of the first in the range.
*/


void ImmutableIndexWriter::Add(Buffer key, Buffer value) {
  record_buffer.push_back(std::pair<Buffer,Buffer>(key,value));
  data_in_buffer++;

  if(data_in_buffer >= chunk_size)
    FlushBuffer();
}

void ImmutableIndexWriter::FlushBuffer() {
  if(record_buffer.size() == 0)
    return;

  vector<offset_t> value_offsets;

  // write values and memorize offsets
  for(WriteBuffer::iterator i = record_buffer.begin();
    i != record_buffer.end(); ++i) {
    void* target = WriteData(i->second, RECORD_TYPE_VALUE);
    value_offsets.push_back(storage.pointer2offset(target));
  }

  // write offset array
  void* offsets_target = WriteData(Buffer(&value_offsets[0],
      value_offsets.size() * sizeof(offset_t)), RECORD_TYPE_OFFSETS);
  offset_t offsets_offset = storage.pointer2offset(offsets_target);

  // write keys
  for(WriteBuffer::iterator i = record_buffer.begin();
    i != record_buffer.end(); ++i) {
    WriteData(i->first, RECORD_TYPE_KEY);
  }

  // memorize first key and offset to values
  index_keys.push_back(record_buffer.begin()->first);
  index_offsets.push_back(offsets_offset);

  record_buffer.clear();
  data_in_buffer = 0;
}

void* ImmutableIndexWriter::WriteData(Buffer data, char type) {
  void* location = storage.getFreeSpace(data.size);
  memcpy(location, data.data, data.size);
  storage.frameData(location, data.size, type);
  return location;
}

void ImmutableIndexWriter::Finalize() {
  FlushBuffer();

  if (index_offsets.size() > 0) {
    WriteData(Buffer(&index_offsets[0],
        index_offsets.size() * sizeof(offset_t)), RECORD_TYPE_INDEX_OFFSETS);

    for(vector<Buffer>::iterator i = index_keys.begin(); i != index_keys.end(); i++) {
      WriteData(*i, RECORD_TYPE_INDEX_KEY);
    }
  } else {
    WriteData(Buffer::Empty(), RECORD_TYPE_INDEX_OFFSETS);
  }

  WriteData(Buffer::Empty(), RECORD_TYPE_FILE_FOOTER);

  storage.commit();
  storage.close();
}

ImmutableIndexIterator::ImmutableIndexIterator(const SequentialFile& file, bool end)
  : file(file), offset_table(0), key(file.Last()) {
  if (!end) {
    findNextOffsetTable(file.First());
  }
}

ImmutableIndexIterator::ImmutableIndexIterator(const ImmutableIndexIterator& o)
  : file(o.file), offset_table(o.offset_table), key(o.key), key_no(o.key_no) {}

ImmutableIndexIterator::ImmutableIndexIterator(SequentialFile& file, offset_t* table, SequentialFile::iterator i, int n)
  : file(file), offset_table(table), key(i), key_no(n) {}

void ImmutableIndexIterator::operator ++ () {
  ASSERT_TRUE(key.IsValid());
  ASSERT_TRUE(key.GetNext() != NULL);
  ++key_no;

  if (key.IsType(RECORD_TYPE_INDEX_OFFSETS)) {
    key = file.Last();
  } else if (!key.IsType(RECORD_TYPE_KEY)) {
    findNextOffsetTable(key);
  }
}

std::pair<Buffer,Buffer> ImmutableIndexIterator::operator * () {
  ASSERT_TRUE(offset_table != NULL);
  ASSERT_TRUE(key.GetRecord()->getType() == RECORD_TYPE_KEY);
  SequentialFile::Record* key_rec = key.GetRecord();
  SequentialFile::Record* value_rec = file.at(offset_table[key_no]).GetRecord();

  return std::pair<Buffer,Buffer>(
    Buffer(key_rec->getPayload(), key_rec->getPayloadSize()),
    Buffer(value_rec->getPayload(), value_rec->getPayloadSize()));
}

bool ImmutableIndexIterator::operator != ( const ImmutableIndexIterator& other ) {
  return key != other.key;
}

bool ImmutableIndexIterator::operator == ( const ImmutableIndexIterator& other ) {
  return key == other.key;
}

void ImmutableIndexIterator::findNextOffsetTable(SequentialFile::iterator it) {
  while (it.GetNext() && !it.IsType(RECORD_TYPE_OFFSETS))
    ;

  if (it.IsValid()) {
    ASSERT_TRUE(it.GetRecord()->getType() == RECORD_TYPE_OFFSETS);
    offset_table = (offset_t*)it.GetRecord()->getPayload();
    it.GetNext();
    ASSERT_TRUE(it.GetRecord()->getType() == RECORD_TYPE_KEY);
    key = it;
    key_no = 0;
  }
  else {
    offset_table = NULL;
    key = it;
    key_no = -1;
  }
}
