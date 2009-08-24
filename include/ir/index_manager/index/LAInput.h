#ifndef LAINPUT_H
#define LAINPUT_H

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class LAInputUnit
{
public:
	LAInputUnit():termId_(0),wordOffset_(0),byteOffset_(0){}
	LAInputUnit(const LAInputUnit& clone):termId_(clone.termId_),wordOffset_(clone.wordOffset_),byteOffset_(clone.byteOffset_){}
	~LAInputUnit(){}
public:
	unsigned int termId_;
	unsigned int wordOffset_;
	unsigned int byteOffset_;
};

typedef std::deque<LAInputUnit> LAInput;
}

NS_IZENELIB_IR_END

#endif
