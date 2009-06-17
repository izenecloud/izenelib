#ifndef FORWARDINDEX_H
#define FORWARDINDEX_H

#include <util/DynamicArray.h>

#include <deque>

using namespace izenelib::util;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef std::deque<std::pair<unsigned int, unsigned int> > ForwardIndexOffset;
typedef DynamicArray<ForwardIndexOffset*, Const_NullValue<ForwardIndexOffset*> > ForwardIndex;

}

NS_IZENELIB_IR_END

#endif
