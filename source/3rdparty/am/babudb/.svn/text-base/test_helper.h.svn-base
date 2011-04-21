// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Licensed under the BSD License, see LICENSE file for details.

#ifndef BABUDB_TEST_HELPER_H
#define BABUDB_TEST_HELPER_H

#include <string>
using std::string;

#include "babudb/buffer.h"
#include "babudb/log/log_section.h"

#define DUMMY_OPERATION_TYPE 5

class DummyOperation : public babudb::Serializable {
public:
	DummyOperation(int i) : value(i) {}

	virtual void Serialize(const babudb::Buffer& data) const {
		*((int*)data.data) = value;
	}

	virtual size_t GetSize() const {
		return sizeof(int);
	}

  virtual int GetType() const { 
    return DUMMY_OPERATION_TYPE; 
  }

	DummyOperation& Deserialize(const babudb::Buffer& data) {
		value = *((int*)data.data);
		return *this;
	}

	int value;
};

#endif
