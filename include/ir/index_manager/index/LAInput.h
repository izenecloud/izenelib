#ifndef LAINPUT_H
#define LAINPUT_H

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class LAInputUnit
{
public:
	LAInputUnit():termId_(0),offset_(0){}
	LAInputUnit(const LAInputUnit& clone):termId_(clone.termId_),offset_(clone.offset_){}
	~LAInputUnit(){}
public:
	unsigned int termId_;
	unsigned int offset_;
};

typedef std::deque<LAInputUnit> LAInput;
}

NS_IZENELIB_IR_END

#endif
