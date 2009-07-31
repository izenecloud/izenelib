#ifndef FORWARDINDEX_H
#define FORWARDINDEX_H

#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef std::deque<std::pair<unsigned int, unsigned int> > ForwardIndexOffset;
typedef rde::hash_map<unsigned int, ForwardIndexOffset* > ForwardIndex;

}

NS_IZENELIB_IR_END

#endif
