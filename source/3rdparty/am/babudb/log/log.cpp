// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "log/log_iterator.h"

#include "log/log.h"
#include "log/log_section.h"
#include "log/log_storage.h"
#include "util.h"
using namespace babudb;

#include <algorithm>
#include <sstream>
using namespace std;

#include <platform/directory_walker.h>
#include <platform/assert.h>
#include <platform/disk_operations.h>

using namespace YIELD;

Log::Log(Buffer data) : tail(NULL), name_prefix("") {  // volatile in-memory
  LogStorage* storage = new VolatileLogStorage(data);
  tail = new LogSection(storage, 1);
  sections.push_back(tail);
}

Log::Log(const string& name_prefix) : tail(NULL), name_prefix(name_prefix) {}

Log::~Log() {
  if (tail != NULL) {
    close();
  }
  for(vector<LogSection*>::iterator i = sections.begin(); i != sections.end(); ++i)
    delete *i;
}

void Log::close() {
  advanceTail();
}

class LSNBefore {
public:
  typedef pair<YIELD::Path, lsn_t> vector_entry;
  bool operator () (const vector_entry& l, const vector_entry& r) { return l.second < r.second; }
};

typedef vector< pair<YIELD::Path, lsn_t> > DiskSections;

static DiskSections scanAvailableLogSections(const string& name_prefix) {
  DiskSections result;

  pair<YIELD::Path,YIELD::Path> prefix_parts = YIELD::Path(name_prefix).split();
  YIELD::DirectoryWalker walker(prefix_parts.first);

  while(walker.hasNext()) {
    lsn_t lsn;
    auto_ptr<YIELD::DirectoryEntry> entry = walker.getNext();

    if(matchFilename(entry->getPath(), prefix_parts.second.getHostCharsetPath(), "log", lsn))
      result.push_back(make_pair(entry->getPath(), lsn));
  }

  std::sort(result.begin(),result.end(),LSNBefore());
  return result;
}

// Rename log sections with LSNs smaller than to_lsn
void Log::cleanup(lsn_t to_lsn, const string& obsolete_prefix) {
  DiskSections disk_sections = scanAvailableLogSections(name_prefix);  // sorted by LSN

  for (DiskSections::iterator i = disk_sections.begin(); i != disk_sections.end(); ++i) {
    DiskSections::iterator next = i; next++;

    if (next != disk_sections.end() && next->second <= to_lsn) {
      pair<YIELD::Path,YIELD::Path> parts = i->first.split();
      YIELD::DiskOperations::rename(i->first, obsolete_prefix + parts.second.getHostCharsetPath());
    }
  }
}

// Loads all log sections with LSNs larger than min_lsn. Load them in order and check
// for continuity.
void Log::loadRequiredLogSections(lsn_t min_lsn) {
  loadSections(min_lsn, false);
}

void Log::loadAllSections(bool read_write) {
  loadSections(0, read_write);
}

void Log::loadSections(lsn_t min_lsn, bool read_write) {
  ASSERT_TRUE(sections.size() == 0);  // otherwise somebody called startup() twice
  DiskSections disk_sections = scanAvailableLogSections(name_prefix);  // sorted by LSN

  for (DiskSections::iterator i = disk_sections.begin(); i != disk_sections.end(); ++i) {
    DiskSections::iterator next = i; next++;

    if(next == disk_sections.end() || (min_lsn + 1) < next->second) {
      LogStorage* file = NULL;
      if (read_write) {
        file = PersistentLogStorage::Open(i->first);
      } else {
        file = PersistentLogStorage::OpenReadOnly(i->first);
      }
      LogSection* section = new LogSection(file, i->second); // repairs if not graceful

      if (!section->empty()) { // check if there is a LSN in this section
        sections.push_back(section);
      } else {
        delete section;
      }
    }
  }
}

LogSection* Log::getTail(babudb::lsn_t next_lsn) {
  if(tail == NULL) {
    LogStorage* storage = NULL;
    // TODO: hack, refactor!
    if (!name_prefix.empty()) {
      std::ostringstream section_name;
      section_name << name_prefix << "_" << next_lsn << ".log";
      storage = PersistentLogStorage::Open(section_name.str());
    } else {
      storage = new VolatileLogStorage(1024);
    }
    tail = new LogSection(storage, next_lsn);
    sections.push_back(tail);
  }

  return tail;
}

void Log::advanceTail() {
  if(tail) {
    if(tail->isWritable()) {
      tail->truncate();
    }
  }
  tail = NULL;
}

Log::iterator Log::First() {
  return LogIterator::First(LogSectionIterator::First(sections));
}

Log::iterator Log::Last(){
  return LogIterator::Last(LogSectionIterator::Last(sections));
}
