// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// Bundles ImmutableIndices and LogIndices into one consistent index

#ifndef MergedIndex_H
#define MergedIndex_H

#include <memory>
#include <utility>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include "buffer.h"
#include "key.h"

namespace babudb {
class LogSection;
class LogIndex;
class ImmutableIndex;
class KeyOrder;
class LookupIterator;
class Log;

class MergedIndex {
public:
  explicit MergedIndex(const std::string& name, const KeyOrder& order);
  ~MergedIndex();

  lsn_t GetLastPersistentLSN();
  // Rename obsolete immutable indices
  void Cleanup(const string& to);
  void Snapshot(lsn_t current_lsn);
  LookupIterator GetSnapshot(lsn_t snapshot_lsn);

  typedef map<Buffer,Buffer,SimpleMapCompare> ResultMap;
  Buffer Lookup(const Buffer& key);
  LookupIterator Lookup(const Buffer&, const Buffer&);

  void Add(const Buffer&, const Buffer&);
  void Remove(const Buffer&);

  const KeyOrder& getOrder() { return order; }
  ImmutableIndex* GetBase()  { return immutable_index; }

private:
  LogIndex* tail;
  vector<LogIndex*> log_indices;  // we currently use only one overlay
  ImmutableIndex* immutable_index;
  string name_prefix;
  const KeyOrder& order;
};

};

#endif
