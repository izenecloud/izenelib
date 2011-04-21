// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)


#include "log/record_iterator.h"
#include "log/record_frame.h"
#include <buffer.h>

#include <platform/assert.h>
using namespace YIELD;
using namespace babudb;

RecordIterator::RecordIterator()
    : current(NULL), region_start(NULL),
      region_size(0) {}

RecordIterator::RecordIterator(const RecordIterator& other)
	: current(other.current), region_start(other.region_start),
    region_size(other.region_size) {}

RecordIterator::RecordIterator(void* start, size_t size,
                               RecordFrame* pos)
	: current(pos), region_start(start),
    region_size(size) {}

void* RecordIterator::operator * ()	const {
	ASSERT_TRUE(IsValidPosition(current));
	return current->getPayload();
}

RecordFrame* RecordIterator::GetRecord() const {
	ASSERT_TRUE(IsValidPosition(current));
	return current;
}

Buffer RecordIterator::AsData() const {
	ASSERT_TRUE(IsValidPosition(current));
	return Buffer(current->getPayload(), current->getPayloadSize());
}

unsigned char RecordIterator::GetType() const	{
  return current->getType(); 
}

bool RecordIterator::IsType(unsigned char t) const { 
  return current->getType() == t;
}

size_t RecordIterator::GetSize() const { 
  return current->getPayloadSize();
}

bool RecordIterator::operator != (const RecordIterator& other) const {
	ASSERT_TRUE(IsCompatible(other));
	return current != other.current; 
}

bool RecordIterator::operator == (const RecordIterator& other) const { 
	ASSERT_TRUE(IsCompatible(other));	// maybe you changed the database while iterating?
	return current == other.current;
}

bool RecordIterator::IsCompatible(const RecordIterator& b) const {
  return region_start == b.region_start &&
         region_size == b.region_size;
}

bool RecordIterator::IsValidPosition(RecordFrame* x) const {
  return (char*)x >= (char*)region_start &&
         (char*)x < ((char*)region_start + region_size);
}

char* RecordIterator::RegionEnd() const {
  return ((char*)region_start + region_size);
}

RecordIterator RecordIterator::First(void* start, size_t size) {
	return RecordIterator(start, size, NULL);
}

RecordIterator RecordIterator::Last(void* start, size_t size) {
	return RecordIterator(
      start, size, 
      reinterpret_cast<RecordFrame*>((char*)start + size));
}

bool RecordIterator::IsValid() const {
  return current != NULL && current != (RecordFrame*)RegionEnd();
}

RecordFrame* RecordIterator::GetNext() {
  current = FindNextRecord(current);
	ASSERT_TRUE(ISALIGNED(current, RECORD_FRAME_ALIGNMENT));
	if (current != (RecordFrame*)RegionEnd()) {
		return current;
  } else {
    return NULL;
  }
}

RecordFrame* RecordIterator::GetPrevious() {
  current = FindPreviousRecord(current);
	ASSERT_TRUE(ISALIGNED(current, RECORD_FRAME_ALIGNMENT));
  return current;
}

RecordFrame* RecordIterator::FindNextRecord(RecordFrame* record) const {
  if (record == NULL) {
    return (RecordFrame*)region_start;
  }

	if(record == (RecordFrame*)RegionEnd()) {
		return record;
  }

	record_frame_t* peek = (record_frame_t*)record->getEndOfRecord();

	while((char*)peek < RegionEnd() && *peek == 0)
		peek++;

	ASSERT_TRUE((char*)peek <= RegionEnd());

	record = (RecordFrame*)peek;

	if(peek != (record_frame_t*)RegionEnd()) {
  	ASSERT_TRUE(IsValidPosition(record));
		ASSERT_TRUE(record->isValid());
	}

	return record;
}

RecordFrame* RecordIterator::FindPreviousRecord(RecordFrame* record) const {
  if (record == NULL || record == region_start) {
    return NULL;
  }

	record_frame_t* peek = (record_frame_t*)record - 1;
	ASSERT_TRUE(IsValidPosition((RecordFrame*)peek));

	while((char*)peek > region_start && *peek == 0)
		peek--;

	if(peek == region_start) 
		return NULL;

	RecordFrame* end = (RecordFrame*)peek;
	record = end->getStartHeader();
  
	ASSERT_TRUE(IsValidPosition(record));
	ASSERT_TRUE(record->isValid());
	return record;
}
