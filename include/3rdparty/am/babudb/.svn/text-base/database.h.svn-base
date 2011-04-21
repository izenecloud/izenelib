// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// The core Database class. A set of persistent and volatile key-value indices that
// represent the application-managed log state up to a certain LSN.
//
// The application manages a log of operations that can be identified by their 
// LSNs (log sequence numbers). These operations change the database state via
// Add() and Remove().

// The database maintains a set if indices that are persistent up to a certain LSN and
// extended with volatile overlay indices. After Open() the application has to replay its
// log from GetMinimalLSN() + 1 against the database to re-establish the current state.

// TODO: 
// - implement index with non-unique keys (for full-text indices)
//   can be implemented as non-unique key or multi-value index.
//   needs a Remove(key, value)


#ifndef BABUDB_DATABASE_H
#define BABUDB_DATABASE_H

#include <utility>
#include <map>
#include <vector>
#include <string>
using std::string;

#include "babudb/buffer.h"

namespace babudb {

class KeyOrder;
class MergedIndex;
class IndexCreator;
class LookupIterator;
class IndexMerger;

typedef std::pair<string, const KeyOrder*> IndexDescriptor;

class Database {
 public:
  // Register the indices and open the database
  static Database* Open(const string& name, const std::vector<IndexDescriptor>& indices);
  ~Database();

  // Reopen indices, possible from a more recent, compacted version
  void ReopenIndices();

  void Add(const string& index_name, lsn_t lsn, const Buffer& key, const Buffer& value);
  void Remove(const string& index_name, lsn_t lsn, const Buffer& key);

  // Single key lookup
  Buffer Lookup(const string& index, const Buffer& key);
  // Prefix lookup
  LookupIterator Lookup(const string& index, const Buffer& lower, const Buffer& upper);

  // The next Add or Remove call needs to have change_lsn = GetCurrentlLSN() + 1
  lsn_t GetCurrentLSN() const;
  // Called after Open to find out from where on to replay the log.
  // Also any log merges need to start from here.
  lsn_t GetMinimalPersistentLSN() const;

  // Index versions:
  // - every index can have multiple versions on disk
  // - on startup the latest intact version is read
  // - new versions are created with 
  //   lsn_t snapshot_lsn = GetCurrentLSN();
  //   Snapshot(index_name);  // @snapshot_lsn
  //   // Database can now be written again,
  //   // compaction can be done in parallel
  //   CompactIndex(index_name, snapshot_lsn);
  // - Cleanup: prepends all obsolete index files
  //   with a prefix, so that you can glob and delete them.

  // Snapshot index at current lsn, for later merging
  void Snapshot(const string& index_name);
  // Compact the index snapshot. It is thread-safe in the sense that normal database
  // operations can continue concurrently.
  void CompactIndex(const string& name, lsn_t snapshot_lsn);
  // Cleanup obsolete indices by moving them to a directory from which they can
  // be later deleted
  void Cleanup(const string& obsolete_prefix);

  // Index import/export
  // Get latest versions (characterized by their lsn) of all immutable indices
  std::vector<std::pair<string, lsn_t> > GetIndexVersions();
  // Get paths of used indices
  std::vector<std::pair<string, string> > GetIndexPaths();

  static string GetIndexFile(
      const string& db_name, const string& index_name,
      lsn_t version);                           

  // Import an index version from a file (file will be renamed)
  static bool ImportIndex(const string& database_name,
                          const string& index_name,
                          lsn_t index_version,
                          const string& source_filename,
                          bool delete_source);

private:
  Database(const string& name);
  void CloseIndices();
  void OpenIndices(const std::vector<IndexDescriptor>& index_list);

  std::map<string, MergedIndex*> indices;
  string name;

  lsn_t latest_lsn;  // the latest known LSN in the Database state
  lsn_t minimal_persistent_lsn;  // the minimum persistent LSN in all indices
};

}  // namespace babudb

#endif
