// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Licensed under the BSD License, see LICENSE file for details.

#ifndef BABUDB_LOGITERATOR_H
#define BABUDB_LOGITERATOR_H

#include "babudb/buffer.h"
#include "babudb/log/record_iterator.h"
#include "babudb/log/log_section.h"

#include <vector>

namespace babudb {
class LogSection;

class LogIterator {
public:
  LogIterator(const LogIterator&);
  static LogIterator First(const LogSectionIterator& first_section);
  static LogIterator Last(const LogSectionIterator& last_section);

  Buffer GetNext();
  Buffer GetPrevious();

  bool operator != (const LogIterator&) const;
  bool operator == (const LogIterator&) const;

  Buffer operator * () const;
  Buffer AsData() const {
    return this->operator *();
  }
  Buffer GetOperationWithFrame() const;
  RecordIterator GetRecordIterator() const {
    return record_iterator;
  }
  record_type_t GetType() const;
  bool IsValid() const;

private:
  LogIterator(const LogSectionIterator& current_section);

  LogSectionIterator current_section;
  RecordIterator record_iterator;
};

}

#endif
