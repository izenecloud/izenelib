// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, 2010 Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// The overlay lookup logic

#ifndef BABUDB_LOOKUPITERATOR_H
#define BABUDB_LOOKUPITERATOR_H

#include <vector>
using std::vector;
#include <map>
using std::map;

#include "babudb/key.h"
#include "babudb/buffer.h"

namespace babudb {

class LogIndex;
class ImmutableIndex;
class Buffer;
class ImmutableIndexIterator;

class LookupIterator {
public:
  LookupIterator(const vector<LogIndex*>& idx, ImmutableIndex* iidx, const KeyOrder& order, const Buffer& start_key, const Buffer& end_key);
  LookupIterator(const vector<LogIndex*>& idx, ImmutableIndex* iidx, const KeyOrder& order);
  explicit LookupIterator(const KeyOrder& order);

  ~LookupIterator();

  void operator ++ ();
  std::pair<Buffer,Buffer> operator * ();

  bool hasMore() const;

private:
  void InitializeOverlays(const vector<LogIndex*>&);
  void findMinimalIterator();
  void advanceIterator(int);
  void assureNonDeletedCursor();
  void CheckInvariant();
  void DebugPrint();

  int current_depth;
  vector<map<Buffer,Buffer,MapCompare>::const_iterator> logi_it;  // MSI to LSI
  const KeyOrder& order;
  const Buffer* end_key;
  vector<LogIndex*> logi;
  ImmutableIndex* iidx;
  ImmutableIndexIterator* iidx_it;
};

}

#endif
