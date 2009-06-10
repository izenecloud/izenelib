#ifndef LAINPUT_H
#define LAINPUT_H

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class LAInputUnit
{
public:
	LAInputUnit():termId_(0),startOffset_(0),endOffset_(0){}
	~LAInputUnit(){}
public:
	unsigned int termId_;
	unsigned int startOffset_;
	unsigned int endOffset_;
};

typedef std::deque<LAInputUnit> LAInput;
}

NS_IZENELIB_IR_END

#endif
