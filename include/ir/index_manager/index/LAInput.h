/**
* @file        LAInput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   
*/
#ifndef LAINPUT_H
#define LAINPUT_H

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/*
* @brief  This class is an interface class exposed to the user of indexmanager
* We provide two kinds of input for an analyzed property to set up index:
* forwardindex, which is refered in ForwardIndex.h, and LAInput.
* LAInput is nothing more than a series of tokens with their cooresponding 
* term offset information
* Using LAInput instead of forwardindex is the input to build the index is faster
* because it reduces the data replication. However, in some cased, forwardindex
* is also required by other utilities, such as summarizaion, ... etc, so providing
* two kinds of input form is necessary
*/
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
