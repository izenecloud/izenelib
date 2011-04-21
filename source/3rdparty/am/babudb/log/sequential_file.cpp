// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#define  _CRT_SECURE_NO_WARNINGS

#include "log/sequential_file.h"
#include "log/log_stats.h"
using namespace babudb;

#include <new>
#include <memory>
#include <string>

#include <platform/assert.h>
using namespace YIELD;

#define POSIX  // for O_ definitions from fcntl.h
#include <fcntl.h>

#define FIRST_RECORD_OFFSET		8
#define FIRST_RECORD_ADDRESS	(void*)((unsigned char*)memory->Start() + FIRST_RECORD_OFFSET)


SequentialFile::SequentialFile(LogStorage* m, LogStats* stats ) : memory( m ), stats( stats )
{
	if(stats == NULL)
		this->stats = new LogStats();

	initialize();

	database_version = *(unsigned int*)offset2record( 0 );

	this->stats->log_file_length = (unsigned int)memory->Size();
	this->stats->log_length = (unsigned int)next_write_offset;

	if(next_write_offset == FIRST_RECORD_OFFSET) // new database
		this->stats->gaps_length = this->stats->no_of_gaps = this->stats->no_of_records= 0;
	else
		this->stats->gaps_length = this->stats->no_of_gaps = this->stats->no_of_records = 0xfFFffFF;
}

void SequentialFile::close() {
	memory->Close();
}

// Remove all empty space between records. Untested.
void SequentialFile::compact() {
	stats->no_of_records = 0;
	offset_t to = FIRST_RECORD_OFFSET;

  iterator current = First();
  current.GetNext();
  iterator previous = current;
  int previous_size = 0;
  if (previous.IsValid()) {
    previous_size = previous.GetSize();
  }
	while (current.GetNext()) {
		offset_t from = record2offset(current.GetRecord());
		offset_t to  = record2offset(previous.GetRecord());
		moveRecord(from, to + previous_size);

    previous = current;
    previous_size = previous.GetSize();
		stats->no_of_records++;
	}

	next_write_offset = to;

	stats->log_length = (unsigned int)next_write_offset;
	stats->no_of_gaps = 0;
	stats->gaps_length = 0;
}

offset_t SequentialFile::findNextAllocatedWord(offset_t offset)
{
	record_frame_t* raw = (record_frame_t*)offset2record(offset) - 1;

	while( *raw == 0 && (unsigned char*)raw >= FIRST_RECORD_ADDRESS )
		raw--;

	return record2offset( (Record*)raw );
}

bool SequentialFile::empty() {
	return next_write_offset == FIRST_RECORD_OFFSET;
}

bool SequentialFile::isWritable() {
  return memory->IsWritable();
}

int SequentialFile::initialize()
{
  if (memory->Size() == 0)
    memory->Resize(FIRST_RECORD_OFFSET);

	next_write_offset = (offset_t)memory->Size();
	next_write_offset = findNextAllocatedWord(next_write_offset);

	record_frame_t* raw = (record_frame_t*)offset2record(next_write_offset);

	if(next_write_offset == FIRST_RECORD_OFFSET) {	// this is a new empty file, everything is fine
		unsigned char* start = (unsigned char*)offset2record( 0 );

		if(isWritable())
			*start = SEQUENTIALFILE_DB_VERSION;

		return 0;
	}

	// find the first valid record
  // TODO: skip initial zeros after FIRST_RECORD_ADDRESS

	int lost_records = 0;
	while(raw > (record_frame_t*)FIRST_RECORD_ADDRESS) {
		if(assertValidRecordChain(raw))	// check whether at the current position could be a valid record
			break;

		raw--;
		lost_records = 1;
	}

	raw = raw + 1;

	next_write_offset = record2offset( (SequentialFile::Record*)raw );


	// find end, wipe out half-written records, write end of file marker
	// clean memory after first valid record

	if(isWritable()) {
		for( raw = raw; (char*)raw < memory->End(); raw++ ) {
			*raw = 0;
    }
  }
	rollback();		// remove any non-finalized transactions

	return lost_records;
}

/** Assert that the given position is the end of a valid record and that after it there is one
	more valid record.

	\result Whether the assertion is true.
*/

bool SequentialFile::assertValidRecordChain( void* raw )
{
	SequentialFile::Record* candidate_end = (Record*)raw;
	SequentialFile::Record* candidate_begin = candidate_end->getStartHeader();

	if( (char*)candidate_begin < memory->Start() || (char*)candidate_begin >= memory->End() )
		return false;

	if( candidate_end->mightBeHeader() && candidate_begin->mightBeHeaderOf( candidate_end ) )
	{
		// It could be that we have found a valid record, check whether the next one is valid, too.

		record_frame_t* test = (record_frame_t*)candidate_begin - 1;

		// skip 0s

		while( *test == 0 && test > FIRST_RECORD_ADDRESS)
			test--;

		// we're now at the first non-0 byte, check whether there is a valid record

		SequentialFile::Record* next_end   = (Record*)test;
		SequentialFile::Record* next_start = next_end->getStartHeader();

		if( (void*)next_start < memory->Start() || (void*)next_start >= memory->End() )
			return false;

		if( next_start->mightBeHeaderOf( next_end ) )
			return true;

		// next record would be invalid so this one is a no real record
	}

	return false;
}



void SequentialFile::frameData(void* payload, size_t size, record_type_t type) {
	ASSERT_TRUE(ISALIGNED(payload, RECORD_FRAME_ALIGNMENT));
	Record* record = Record::GetRecord((char*)payload);

	SequentialFile::Record* new_record = new (record)Record(type, size);
	next_write_offset = record2offset( (SequentialFile::Record*)new_record->getEndOfRecord() );
	ASSERT_TRUE(ISALIGNED((void*)next_write_offset, RECORD_FRAME_ALIGNMENT));
}

void* SequentialFile::getFreeSpace(size_t size) {
	if( next_write_offset + size + 32 > memory->Size() )
		enlarge();

	record_frame_t* location = (record_frame_t*)offset2record( next_write_offset );
	ASSERT_TRUE( *location == 0);

	location++;
	return location;
}

void* SequentialFile::append(size_t size, record_type_t type) {
	void* location = getFreeSpace(size);
	frameData(location, size, type);
	return location;
}

void SequentialFile::AppendRaw(void* data, size_t size) {
	record_frame_t* location = (record_frame_t*)getFreeSpace(size);
  location--;  // TODO: clean up
  memcpy(location, data, size);
  Record* new_record = static_cast<Record*>((void*)location);
  ASSERT_TRUE(new_record->isValid());
	next_write_offset = record2offset( (SequentialFile::Record*)new_record->getEndOfRecord() );
	ASSERT_TRUE(ISALIGNED((void*)next_write_offset, RECORD_FRAME_ALIGNMENT));
}

void SequentialFile::moveRecord( offset_t at, offset_t to )
{
	Record* source = offset2record( at );
	unsigned int size = source->GetRecordSize();

	void* dest = offset2record( to );

	memmove( dest, source, size );

	if( to > at ) // move right, overlap or no overlap
		memset( source, 0, (size_t)(to - at) );

	else if( to + size < at ) // move left, no overlap
		memset( source, 0, size );

	else // move left, overlap
		memset( offset2record( to + size ), 0, (size_t)(at - to) );
}

void SequentialFile::commit()
{
	record_frame_t* next_to_current_record = (record_frame_t*)offset2record(next_write_offset);

	next_to_current_record--;
	SequentialFile::Record* current_record = ((Record*)next_to_current_record)->getStartHeader();

	if(current_record->isEndOfTransaction())	// no operations to commit
		return;

	if(current_record->isValid()) {
		current_record->setEndOfTransaction( true );
		writeBack();
	}
	// else: no operations in database
}

unsigned int SequentialFile::rollback() {
	unsigned int rolledback_operations = 0;

	iterator r = Last();
	while (r.GetPrevious()) {
		if (r.GetRecord()->isEndOfTransaction())
			break;

		if (isWritable())
			erase(record2offset(r.GetRecord()));		// works because prev skips 0's

		rolledback_operations++;
	}

	if (r.IsValid()) {
		next_write_offset = record2offset((Record*)r.GetRecord()->getEndOfRecord() );
  } else {
		next_write_offset = FIRST_RECORD_OFFSET;
  }
	return rolledback_operations;
}

void SequentialFile::erase( offset_t offset )
{
	ASSERT_TRUE(isWritable());

	Record* target = offset2record( offset );
	ASSERT_TRUE(target->isValid());
	offset_t end_offset = offset + target->GetRecordSize();

	char* target_end = (char*)target->getEndOfRecord();
	for (char* wiper = (char*)target; wiper < target_end; wiper++) {
		*wiper = 0;
  }

	// fix next_write_offset if it is the last record
	if (end_offset == next_write_offset) {
		iterator i = at(end_offset);
		i.GetPrevious();

		if (!i.IsValid()) {
			next_write_offset = 8;
    } else {
			next_write_offset = record2offset((Record*)i.GetRecord()->getEndOfRecord());
    }
	}

	stats->no_of_gaps += 1;
	stats->gaps_length += target->GetRecordSize();
	stats->no_of_deletors--;
}

void SequentialFile::enlarge()
{
	size_t old_size = memory->Size();
	size_t new_size = (unsigned int)((float)(old_size < 500000 ? old_size * 6 : old_size * 2));

	memory->Resize( new_size );

	stats->log_file_length = (unsigned int)new_size;
}

void SequentialFile::truncate()
{
	size_t new_size = (unsigned int)next_write_offset;
	memory->Resize( new_size );

	stats->log_file_length = (unsigned int)new_size;
}

/** Create in iterator to advance from the given position

	\param rec The record to start from
	\result An iterator to advance
*/

void SequentialFile::setFlush( bool f )
{
//	memory->setFlush( f );
}

void SequentialFile::copyRecord( Record* record, void* destination )
{
	memcpy( destination, record, record->GetRecordSize() );
}

// OffsetPointerConversion

void* SequentialFile::offset2pointer( offset_t offset ) const
{
	return offset2record( offset )->getPayload();
}

offset_t SequentialFile::pointer2offset( void* payload ) const
{
	return record2offset( Record::GetRecord( (char*)payload ) );
}

// conversion helpers

SequentialFile::Record* SequentialFile::offset2record( offset_t offset ) const
{
	return ((Record*) ( offset + (char *)(memory->Start()) ) );
}

offset_t SequentialFile::record2offset( SequentialFile::Record* record ) const
{
	return (offset_t)( (char*)record - (char *)memory->Start() );
}

bool SequentialFile::isValid( SequentialFile::Record* r )
{
	return ((char*)r >= memory->Start() && (char*)r < memory->End() );
}

void SequentialFile::writeBack( Record* record )
{
	memory->WriteBack( record, record->GetRecordSize() );
}

void SequentialFile::writeBack()
{
	memory->WriteBack();
}

#define ACTUAL_START (char*)memory->Start() + FIRST_RECORD_OFFSET
#define ACTUAL_SIZE (size_t)(next_write_offset - FIRST_RECORD_OFFSET)


SequentialFile::iterator SequentialFile::First() const {
	return SequentialFile::iterator::First(ACTUAL_START, ACTUAL_SIZE);
}

SequentialFile::iterator SequentialFile::Last() const {
	return SequentialFile::iterator::Last(ACTUAL_START, ACTUAL_SIZE);
}


SequentialFile::iterator SequentialFile::at( void* payload ) const {
	return SequentialFile::iterator(ACTUAL_START, ACTUAL_SIZE, (Record*)payload);
}

SequentialFile::iterator SequentialFile::at( Record* record ) const {
	return SequentialFile::iterator(ACTUAL_START, ACTUAL_SIZE, record);
}

SequentialFile::iterator SequentialFile::at( offset_t offset ) const {
	return SequentialFile::iterator(ACTUAL_START, ACTUAL_SIZE, offset2record(offset));
}
