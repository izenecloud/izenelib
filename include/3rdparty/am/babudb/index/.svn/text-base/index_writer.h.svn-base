// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Licensed under the BSD License, see LICENSE file for details.

#ifndef BABUDB_IMMUTABLEINDEXWRITER_H
#define BABUDB_IMMUTABLEINDEXWRITER_H

#include <memory>
#include <utility>
#include <vector>

#include "babudb/buffer.h"
#include "babudb/key.h"
#include "babudb/log/sequential_file.h"

namespace babudb {

#define RECORD_TYPE_KEY 1
#define RECORD_TYPE_VALUE 2
#define RECORD_TYPE_OFFSETS 3
#define RECORD_TYPE_INDEX_KEY 4
#define RECORD_TYPE_INDEX_OFFSETS 5
#define RECORD_TYPE_FILE_FOOTER 6

class LogIndex;
class ImmutableIndex;
class KeyOrder;

class ImmutableIndexWriter {
public:
  ImmutableIndexWriter(LogStorage* mm, size_t chunk_size)
    : storage(mm), chunk_size(chunk_size), data_in_buffer(0) {}

  void Add(Buffer key, Buffer value);
  void FlushBuffer();
  void Finalize();

private:
  void* WriteData(Buffer data, char type);
  typedef std::vector<std::pair<Buffer,Buffer> > WriteBuffer;
  WriteBuffer record_buffer;

  SequentialFile storage;
  size_t chunk_size;
  size_t data_in_buffer;

  std::vector<Buffer> index_keys;
  std::vector<offset_t> index_offsets;
};


class ImmutableIndexIterator {
public:
  ImmutableIndexIterator(const SequentialFile& file, bool end);
  ImmutableIndexIterator(const ImmutableIndexIterator& o);
  ImmutableIndexIterator(SequentialFile& file, offset_t* table, SequentialFile::iterator i, int n);

  void operator ++ ();
  std::pair<Buffer,Buffer> operator * ();
  bool operator != (const ImmutableIndexIterator& other );
  bool operator == (const ImmutableIndexIterator& other );

private:
  void findNextOffsetTable(SequentialFile::iterator it);

  const SequentialFile& file;
  offset_t* offset_table;

  SequentialFile::iterator key; // the current key if offset_table != NULL
  int key_no;  // the ordinal number of the current key
};

}

#endif
