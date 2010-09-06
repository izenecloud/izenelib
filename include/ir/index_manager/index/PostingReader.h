/**
* @file        PostingRead.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief posting reader
*/

#ifndef POSTING_READER_H
#define POSTING_READER_H

#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

struct PostingReader
{
public:
    virtual ~PostingReader(){}

    virtual int32_t decodeNext(uint32_t* pPosting,int32_t length) = 0;

    virtual bool decodeNextPositions(uint32_t* pPosting,int32_t length) = 0;

    virtual bool decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs) = 0;

    virtual void resetPosition() = 0;

    virtual docid_t decodeTo(docid_t docID) = 0;

    virtual count_t docFreq()const = 0;

    virtual int64_t getCTF()const = 0;

    virtual count_t getCurTF()const = 0;

    virtual void setFilter(BitVector* pFilter) = 0;
};

}

NS_IZENELIB_IR_END


#endif

