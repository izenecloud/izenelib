// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "merger.h"
using namespace babudb;

#include <yield/platform/memory_mapped_file.h>

IndexMerger::IndexMerger(const string& file_name, const KeyOrder& order) 
    : file_name(file_name), order(order), base(NULL), last_lsn(0), diff(order, 0) {}

IndexMerger::IndexMerger(const string& file_name, const KeyOrder& order, ImmutableIndex* base)
    : file_name(file_name), order(order), base(base), last_lsn(0), diff(order, 0) {}

// Step 2: Fill with data up to a certain LSN
void IndexMerger::Add(lsn_t lsn, const Buffer& key, const Buffer& value) {
  diff.Add(key, value);
  last_lsn = lsn;
}

void IndexMerger::Remove(lsn_t lsn, const Buffer& key) {
  diff.Add(key, Buffer::Deleted());
  last_lsn = lsn;
}

void IndexMerger::Setup() {
  destination.reset(ImmutableIndex::Create(file_name, last_lsn, 64*1024));
  diff_it = diff.begin();

  if (base) {
    base_it = new ImmutableIndex::iterator(base->begin());
  }
}

// Step 3: Merge with old ImmutableIndex or create a new one from scratch
static inline Buffer getKey(LogIndex::iterator it) { return it->first; }
static inline Buffer getKey(ImmutableIndex::iterator it) { return (*it).first; }

void IndexMerger::Proceed(int n_steps) {
  if (!base) {
	  for(int i = 0; i < n_steps; ++i) {
		  if(IsFinished()) {
			  destination->Finalize();
			  break;
		  }
		  else {
			  destination->Add(diff_it->first, diff_it->second);
			  ++diff_it;
			  continue;
		  }
	  }  
  } else {
	  for(int i = 0; i < n_steps; ++i) {
		  if(IsFinished()) {
			  destination->Finalize();
			  break;
		  }
		  else if(*base_it == base->end()) {
			  destination->Add(diff_it->first, diff_it->second);
			  ++diff_it;
			  continue;
		  }
		  else if(diff_it == diff.end()) {
			  destination->Add((*(*base_it)).first,(*(*base_it)).second);
			  ++(*base_it);
			  continue;
		  }

		  if (!order.less(getKey(*base_it), getKey(diff_it)) &&
			    !order.less(getKey(diff_it), getKey(*base_it))) { // equal

			  if(!getKey(diff_it).isDeleted()) 	// diff overwrites key
				  destination->Add(diff_it->first, diff_it->second);

			  ++diff_it;
			  ++(*base_it);

			  continue;
		  }

		  if(order.less(getKey(*base_it), getKey(diff_it))) {
			  destination->Add((*(*base_it)).first,(*(*base_it)).second);
			  ++(*base_it);
			  continue;
		  }

		  if(order.less(getKey(diff_it), getKey(*base_it))) {
			  destination->Add(diff_it->first, diff_it->second);
			  ++diff_it;
			  continue;
		  }

		  FAIL();
	  }

  }
}

bool IndexMerger::IsFinished() {
  if (base) {
  	return *base_it == base->end() && diff_it == diff.end();
  } else {
  	return diff_it == diff.end();
  }
}
