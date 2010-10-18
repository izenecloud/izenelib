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

    virtual int32_t decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& posBufLength, int32_t& posLength) = 0;

    virtual bool decodeNextPositions(uint32_t* pPosting,int32_t length) = 0;

    virtual bool decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting) = 0;

    virtual void resetPosition() = 0;

    virtual docid_t decodeTo(docid_t target, uint32_t* pPosting, int32_t length, int32_t& decodedCount, int32_t& nCurrentPosting) = 0;

    virtual count_t docFreq()const = 0;

    virtual int64_t getCTF()const = 0;

    virtual count_t getCurTF()const = 0;

    virtual void setFilter(BitVector* pFilter) = 0;

protected:
    void growPosBuffer(uint32_t* &pPPosting, int32_t& posBufLength)
    {
        posBufLength = posBufLength << 1;
        pPPosting = (uint32_t*)realloc(pPPosting, posBufLength * sizeof(uint32_t));
    }

};

}

NS_IZENELIB_IR_END


#endif

