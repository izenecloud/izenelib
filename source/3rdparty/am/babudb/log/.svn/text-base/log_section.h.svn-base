// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// The LogSection augments the records of a SequentialFile with LSNs.

#ifndef LOG__LOG_SECTION_H
#define LOG__LOG_SECTION_H

#include <string>
using std::string;
#include <vector>

#include "babudb/log/sequential_file.h"
#include "babudb/buffer.h"

#define LSN_RECORD_TYPE 0
#define USER_RECORD_TYPE 1

namespace babudb {

class LogStorage;

class Serializable {
public:
  virtual size_t GetSize() const = 0;
  virtual void Serialize(const Buffer& buffer) const = 0;
  virtual int GetType() const { return 0; }
  virtual ~Serializable() {}
};

class LogSection : public SequentialFile {
public:
	LogSection(LogStorage*, lsn_t first);

	lsn_t getFirstLSN() const;

  // Append entry, start a new transaction if necessary
	void Append(const Serializable& entry);
  // Make the current transaction durable
	void Commit();
  void Erase(const iterator& it);

private:
	lsn_t first_lsn; // the first lsn in this file
};

class LogSectionIterator {
public:
  LogSectionIterator(const LogSectionIterator& it); 
  void operator = (const LogSectionIterator& other);

  static LogSectionIterator First(std::vector<LogSection*>& sections);
  static LogSectionIterator Last(std::vector<LogSection*>& sections);
  
	LogSection* GetNext();
	LogSection* GetPrevious();
  bool IsValid() const;
	bool operator != (const LogSectionIterator& other) const;
	bool operator == (const LogSectionIterator& other) const;

	LogSection* operator * ()	const;

private:
  LogSectionIterator(std::vector<LogSection*>& sections,
                     std::vector<LogSection*>::iterator current);
  std::vector<LogSection*>& sections;
  std::vector<LogSection*>::iterator current_section;
};

}

#endif
