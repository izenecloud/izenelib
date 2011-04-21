// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// A database for string-type data. Also acts as an example on
// how to put to together babudb's classes to a complete 
// embedded database.

#ifndef BABUDB_STRING_DB_H
#define BABUDB_STRING_DB_H

#include "babudb/profiles/string_key.h"

#include <string>
using std::string;

namespace babudb {

class Database;
class Log;

class StringDB {
public:
  static StringDB* Open(const string& name, const std::vector<string>& indices);
  ~StringDB();

  void Add(const string& index_name, const string& key, const string& value);
  void Remove(const string& index_name, const string& key);
  void Commit();

  string Lookup(const string& index, const string& key);
  // TODO: hide the LookupIterator
  LookupIterator Lookup(const string& index, const string& lower, const string& upper);

  // Merge the log and indices; truncate the log; prefix the obsolete data with "to"
  void Compact(const string& to);

private:
  StringDB(const string& name);
  StringOrder key_order;

  Database* db;
  Log* log;
  std::vector<string> index_names;
  string name;
};

}

#endif
