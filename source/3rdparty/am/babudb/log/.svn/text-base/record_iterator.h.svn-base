// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// A RecordIterator is a container for the state necessary to iterate over the Records in a SequentialFile

#ifndef LOG__RECORD_ITERATOR_H
#define LOG__RECORD_ITERATOR_H

#include <cstddef>
#include "babudb/log/record_frame.h"

namespace babudb {

class RecordFrame;
class Buffer;
class SequentialFile;

class RecordIterator
{
public:
	RecordIterator(const RecordIterator& other);
	RecordIterator();

	static RecordIterator First(void* start, size_t size);
	static RecordIterator Last(void* start, size_t size);

  RecordFrame* GetNext();
  RecordFrame* GetPrevious();

	bool operator != (const RecordIterator& other) const;
	bool operator == (const RecordIterator& other) const;

  // The following operations refer to the current record as
  // returned by GetNext/GetPrevious
	void* operator * ()	const;
	size_t GetSize() const;
	record_type_t GetType() const;
	bool IsType(record_type_t t) const;
	RecordFrame* GetRecord() const;
	Buffer AsData() const;
  bool IsValid() const;

protected:
  friend class SequentialFile;
	RecordIterator(void* start, size_t size, RecordFrame* pos);

	RecordFrame* FindNextRecord(RecordFrame*) const;
	RecordFrame* FindPreviousRecord(RecordFrame*) const;

  bool IsCompatible(const RecordIterator& other) const;
  bool IsValidPosition(RecordFrame* pos) const;
  char* RegionEnd() const;

	RecordFrame* current;
	void* region_start;
	size_t region_size;
};

}

#endif
