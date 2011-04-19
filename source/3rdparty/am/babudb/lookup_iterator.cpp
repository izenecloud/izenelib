// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, 2010 Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "lookup_iterator.h"
#include "key.h"

#include "index/index.h"
#include "index/index_writer.h"
#include "log_index.h"

#include "platform/memory_mapped_file.h"
#include "platform/assert.h"
using namespace YIELD;

using namespace babudb;

/** LookupIterator implements a parallel lookup on all overlay LogIndices
    and the current ImmutableIndex.

   It maintains a list of cursors, and an index "current_depth" that points
  to the iterator that is the current cursor position. The lower the depth,
  the more significant the index is. The ImmutableIndex is treated in a
  special manner.

  From that, an invariants results:
  - all cursors are >= the cursor at current_depth
  - all cursors are >= start_key and < end_key
  - if there are multiple cursors at the current position,
    current_depth points to the most-significant one

  The implementation has to make sure that Deleted entries are skipped.
  It also needs to advance any "shadowed" cursors that point to the same key,
  but are less signifcant than the current_depth cursor.
*/

#define IMMUTABLE_INDEX -1

LookupIterator::LookupIterator(
    const vector<LogIndex*>& idx, ImmutableIndex* iidx, const KeyOrder& order,
    const Buffer& start_key, const Buffer& end_key)
    : current_depth(0), order(order), end_key(&end_key), iidx(iidx), iidx_it(NULL) {

  // Wind overlay iterators to their minimum position
  for(vector<LogIndex*>::const_iterator i = idx.begin(); i != idx.end(); ++i) {
    LogIndex::iterator c = (*i)->find(start_key);
    if(c != (*i)->end() && !order.less(end_key, (*c).first)) {
      logi.push_back(*i);
      logi_it.push_back(c);
    }
  }

  if(iidx) {
    ImmutableIndexIterator c = iidx->Find(start_key);
    if(c != iidx->end() && !order.less(end_key, (*c).first))
      iidx_it = new ImmutableIndexIterator(c);
  }

  if(!hasMore())
    return;
  
  // each iterator is now pointing to a start_key <= key <= end_key

  // find the smallest iterator
  findMinimalIterator();
  DebugPrint();
  // if it is deleted advance it (and any iterators it may shadow)
  assureNonDeletedCursor();
  
  DebugPrint();
}

LookupIterator::LookupIterator(
    const vector<LogIndex*>& idx, ImmutableIndex* iidx, const KeyOrder& order)
    : order(order), end_key(NULL), iidx(iidx), iidx_it(NULL) {

  // Wind overlay iterators to their minimum position 
  for(vector<LogIndex*>::const_iterator i = idx.begin(); i != idx.end(); ++i) {
    LogIndex::iterator c = (*i)->begin();

    if(c != (*i)->end()) {
      logi.push_back(*i);
      logi_it.push_back(c);
    }
  }

  if(iidx) {
    ImmutableIndexIterator c = iidx->begin();
    if(c != iidx->end())
      iidx_it = new ImmutableIndexIterator(c);
  }
  
  if(!hasMore())
    return;
  
  // find the smallest iterator
  findMinimalIterator();
  // if it is deleted advance it (and any iterators it may shadow)
  assureNonDeletedCursor();
  
  DebugPrint();
}

LookupIterator::LookupIterator(const KeyOrder& order)
    : order(order), iidx(NULL), iidx_it(NULL) {}

LookupIterator::~LookupIterator() {
  delete iidx_it;
}

void LookupIterator::findMinimalIterator() {
  // Find the smallest most significant key
  current_depth = 0;
  for (int i = 0; i < (int)logi_it.size(); ++i) {
    ASSERT_TRUE(logi_it[i] != logi[i]->end());
    if (order.less(logi_it[i]->first, logi_it[current_depth]->first)) {
      current_depth = i;
    }
  }

  if(iidx_it) {
    if (logi_it.size() == 0 || order.less((**iidx_it).first, logi_it[current_depth]->first)) {
       current_depth = IMMUTABLE_INDEX;
    }
  } else {
    if (logi_it.size() == 0) {
     // We should never hit this because we always check hasMore() before
      FAIL();
    }
  }
}

void LookupIterator::advanceIterator(int depth) {
  if (depth != IMMUTABLE_INDEX) {
    ++logi_it[depth];

    // are we done with that slice? remove it...
    if (logi_it[depth] == logi[depth]->end() ||
        (end_key != NULL && order.less(*end_key, logi_it[depth]->first))) {
      logi_it.erase(logi_it.begin() + depth);
      logi.erase(logi.begin() + depth);
    }
  } else {
    ++(*iidx_it);

    if (*iidx_it == iidx->end() ||
        (end_key != NULL && order.less(*end_key, (**iidx_it).first))) {
      delete iidx_it;
      iidx_it = NULL;
    }
  }
}

void LookupIterator::CheckInvariant() {
#ifdef _DEBUG
  for (int i = 0; i < (int)logi_it.size(); ++i) {
    ASSERT_TRUE(logi_it[i] != logi[i]->end());
    if (i == current_depth)
      continue;
    if (i < current_depth) { // more significant indices must be larger than current
      if (current_depth != IMMUTABLE_INDEX) {
        ASSERT_TRUE(order.less(logi_it[current_depth]->first, logi_it[i]->first));
      } else {
        ASSERT_TRUE(order.less((**iidx_it).first, logi_it[i]->first));
      }
    } else { // less significant must be larger or equal than current (current may shadow them)
      if (current_depth != IMMUTABLE_INDEX) {
        ASSERT_TRUE(!order.less(logi_it[i]->first, logi_it[current_depth]->first));
      } else {
        ASSERT_TRUE(!order.less(logi_it[i]->first, (**iidx_it).first));
      }
    }
  }
#endif
}

void LookupIterator::DebugPrint() {
#if 0
#include <Windows.h>
void LookupIterator::DebugPrint() {
  for (int i = 0; i < (int)logi_it.size(); ++i) {
    if (i == current_depth)
      OutputDebugString(" (>) ");
    string key((char*)logi_it[i]->first.data, logi_it[i]->first.size);
    if (logi_it[i]->second.data != NULL) {
      string value((char*)logi_it[i]->second.data, logi_it[i]->second.size);
      OutputDebugString((key + ":" + value + " |\t").c_str());
    }
    else {
      OutputDebugString((key + ": XXX |\t").c_str());
    }
  }

  if (iidx_it) {
    if (-1 == current_depth)
      OutputDebugString(" (*) ");
    string key((char*)(**iidx_it).first.data, (**iidx_it).first.size);
    if ((**iidx_it).second.data != NULL) {
      string value((char*)(**iidx_it).second.data, (**iidx_it).second.size);
      OutputDebugString((key + ":" + value + " |\t").c_str());
    }
    else {
      OutputDebugString((key + ": XXX |\t").c_str());
    }
    
  }

  OutputDebugString("\n");
#endif
}

void LookupIterator::operator ++ () {
  CheckInvariant();
  
  DebugPrint();
  // 1. The current cursor might hide cursors that point to the same key,
  // but are less significant ('shadowed'). Advance all shadowed iterators
  // that are equal to the current cursor.
  if (current_depth != IMMUTABLE_INDEX) {
    for (int i = current_depth + 1; i < (int)logi_it.size(); ++i) {
      ASSERT_TRUE(logi_it[i] != logi[i]->end());
      // Compare against current_depth (which may be the immutable index)
      if (current_depth != IMMUTABLE_INDEX) {
        if (!order.less(logi_it[current_depth]->first, logi_it[i]->first)) {
          advanceIterator(i);
        }
      } else {
        if (!order.less((**iidx_it).first, logi_it[i]->first)) {
          advanceIterator(i);
        }
      }
    }
    // Advance also the immutable index iterator if it is shadowed
    if (current_depth != IMMUTABLE_INDEX && iidx_it &&
        !order.less(logi_it[current_depth]->first, (**iidx_it).first)) {
      advanceIterator(IMMUTABLE_INDEX);
    }
  }

  DebugPrint();
  // 1. Advance the current iterator and remove it if it is out of bounds
  advanceIterator(current_depth);
  // Now current_depth does not point anymore to the smallest iterator
  DebugPrint();



  if(!hasMore())
    return;

  // each iterator is now pointing to a start_key <= key <= end_key

  // 3. find the next position
  findMinimalIterator();
  
  DebugPrint();

  // 4. Advance over deletor entries
  assureNonDeletedCursor();

  CheckInvariant();
}


void LookupIterator::assureNonDeletedCursor() {
  if (current_depth != IMMUTABLE_INDEX) {
    if (logi_it[current_depth]->second.isDeleted()) {
      this->operator ++();
    }
  } else {
    if ((**iidx_it).second.isDeleted()) {
      this->operator ++();
    }
  }
}

std::pair<Buffer,Buffer> LookupIterator::operator * () {
  if(current_depth != IMMUTABLE_INDEX) {
    ASSERT_TRUE(logi_it[current_depth] != logi[current_depth]->end());
    ASSERT_FALSE(logi_it[current_depth]->second.isDeleted());
    return std::make_pair(logi_it[current_depth]->first, logi_it[current_depth]->second);
  } else {
    ASSERT_TRUE(*iidx_it != iidx->end());
    ASSERT_FALSE((**iidx_it).second.isDeleted());
    return std::make_pair((**iidx_it).first,(**iidx_it).second);
  }
}

bool LookupIterator::hasMore() const {
  return logi_it.size() > 0 || iidx_it != NULL;
}

