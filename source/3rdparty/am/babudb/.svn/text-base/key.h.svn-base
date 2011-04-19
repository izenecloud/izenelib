// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#ifndef BABUDB_KEYORDER_H
#define BABUDB_KEYORDER_H

#include "babudb/buffer.h"

namespace babudb {

class KeyOrder {
public:
	/* a strict less than operator < */
	virtual bool less(const Buffer& l, const Buffer& r) const = 0;
};

/* The operator that implements less with prefix matches for std::map using KeyOrder */
class MapCompare {
public:
	explicit MapCompare(const KeyOrder& order) : order(order) {}
	bool operator () (const Buffer& l, const Buffer& r) const {
		return order.less(l,r);
	}

private:
	const KeyOrder& order;
};


/*inline bool MapCompare::operator () (const std::pair<Buffer,bool>& l, const std::pair<Buffer,bool>& r) const {
//	ASSERT_FALSE(l.second && r.second, "Map Wrong");

	if(l.second && order.match(r.first,l.first)) // l is prefix key
		return false;
	else if(r.second && order.match(l.first,r.first)) // r is prefix key
		return false;
	else
		return order.less(l.first,r.first);
}
*/

/* The operator that implements simple less for std::map using KeyOrder */
class SimpleMapCompare {
public:
	explicit SimpleMapCompare(KeyOrder& order) : order(order) {}
	bool operator () (const Buffer& l, const Buffer& r) const {
		return order.less(l,r);
	}

private:
	KeyOrder& order;
};

}

#endif

