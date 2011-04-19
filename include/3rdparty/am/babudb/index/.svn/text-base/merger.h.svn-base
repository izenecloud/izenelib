// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#ifndef INDEXMERGER_H
#define INDEXMERGER_H

#include "index.h"
#include "index_writer.h"
#include "log_index.h"

#include <memory>
#include <string>
using std::string;

namespace babudb {

// Creates a persistent index from a LogIndex
class IndexMerger {
public:
  // Step 1: Create the merger
  explicit IndexMerger(const string&, const KeyOrder&);
  explicit IndexMerger(const string&, const KeyOrder&, ImmutableIndex* base);

  // Step 2: Fill with data up to a certain LSN
  void Add(lsn_t lsn, const Buffer&, const Buffer&);
  void Remove(lsn_t lsn, const Buffer&);

  // Step 3: Merge with old ImmutableIndex or create a new one from scratch
  void Setup();
  void Proceed(int steps);
  bool IsFinished();

  void Run() {
    Setup();
    while (!IsFinished()) {
      Proceed(100);
    }
  }

protected:
  string file_name;
  const KeyOrder& order;
  ImmutableIndex* base; 
  lsn_t last_lsn;
  LogIndex diff;

  ImmutableIndex::iterator* base_it;
  LogIndex::iterator diff_it;
  
  std::auto_ptr<ImmutableIndexWriter> destination;
};

}

#endif
