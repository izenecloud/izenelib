// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Licensed under the BSD License, see LICENSE file for details.

#include "log/log_iterator.h"
#include "log/log_section.h"
#include "log/sequential_file.h"

#include <platform/assert.h>

#include <vector>

namespace babudb {

LogIterator::LogIterator(const LogIterator& o)
  : current_section(o.current_section), record_iterator(o.record_iterator) {}

LogIterator::LogIterator(const LogSectionIterator& current_section)
  : current_section(current_section) {}

LogIterator LogIterator::First(const LogSectionIterator& first_section) {
  LogIterator it(first_section);
  if (first_section.IsValid()) {
    it.record_iterator = (*first_section)->First();
  }
  return it;
}

LogIterator LogIterator::Last(const LogSectionIterator& last_section) {
  LogIterator it(last_section);
  if (last_section.IsValid()) {
    it.record_iterator = (*last_section)->Last();
  }
  return it;
}

Buffer LogIterator::GetNext() {
  if (!record_iterator.GetNext()) {
    if (!current_section.GetNext()) {
      return Buffer::Deleted();
    } else {
      record_iterator = (*current_section)->First();
      return GetNext();
    }
  } else {
    ASSERT_TRUE(record_iterator.IsValid());
    return record_iterator.AsData();
  }
}

Buffer LogIterator::GetPrevious() {
  if (!record_iterator.GetPrevious()) {
    if (!current_section.GetPrevious()) {
      return Buffer::Deleted();
    } else {
      record_iterator = (*current_section)->Last();
      return GetPrevious();
    }
  } else {
    ASSERT_TRUE(record_iterator.IsValid());
    return record_iterator.AsData();
  }
}

bool LogIterator::IsValid() const {
  return record_iterator.IsValid();
}

bool LogIterator::operator != (const LogIterator& other) const {
  return current_section != other.current_section || record_iterator != other.record_iterator;
}

bool LogIterator::operator == (const LogIterator& other) const {
  return current_section == other.current_section && record_iterator == other.record_iterator;
}

Buffer LogIterator::operator * () const {
  return record_iterator.AsData();
}

Buffer LogIterator::GetOperationWithFrame() const {
  return Buffer(record_iterator.GetRecord(), record_iterator.GetRecord()->GetRecordSize());
}

record_type_t LogIterator::GetType() const {
  return record_iterator.GetType();
}

}
