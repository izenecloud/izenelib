/**
* @file        PostingRead.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief posting reader
*/

#ifndef POSTING_READER_H
#define POSTING_READER_H

#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/index/CompressParameters.h>

#include <cassert>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

struct PostingReader
{
public:
    virtual ~PostingReader(){}

    virtual int32_t decodeNext(uint32_t* pPosting,int32_t length) = 0;

    virtual int32_t decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& posBufLength, int32_t& posLength) = 0;

    virtual bool decodeNextPositions(uint32_t* pPosting,int32_t length) = 0;

    virtual bool decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, int32_t decodeLength, int32_t& nCurrentPPosting) = 0;

    virtual bool decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting) = 0;

    virtual void resetPosition() = 0;

    virtual docid_t decodeTo(docid_t target, uint32_t* pPosting, int32_t length, int32_t& decodedCount, int32_t& nCurrentPosting) = 0;

    virtual count_t docFreq()const = 0;

    virtual int64_t getCTF()const = 0;

    virtual count_t getCurTF()const = 0;

    virtual void setFilter(BitVector* pFilter) = 0;

protected:
    /**
     * Grow the position buffer size until @p minBufLength is reached.
     * @param pPPosting reference to position buffer, it stores the grown buffer on return
     * @param posBufLength position buffer length, it stores grown buffer size on return
     * @param minBufLength minimum buffer length, @p posBufLength would not be less than this value on return
     */
    void growPosBuffer(uint32_t* &pPPosting, int32_t& posBufLength, int32_t minBufLength) const
    {
        assert(posBufLength < minBufLength);

        while(posBufLength < minBufLength)
            posBufLength <<= 1;

        pPPosting = (uint32_t*)realloc(pPPosting, posBufLength * sizeof(uint32_t));

        assert(posBufLength >= minBufLength);
    }

    /**
     * Grow the position buffer size @p posBufLength until an upper bound of @p minBufLength is reached.
     * @param pPPosting reference to position buffer, it stores the grown buffer on return
     * @param posBufLength position buffer length, it stores grown buffer size on return
     * @param minBufLength minimum buffer length, @p posBufLength would not be less than the upper bound of this value on return
     * @note the upper bound is met as S9 and S16 decoding need at least 28 extra bytes for decompression.
     */
    void ensurePosBufferUpperBound(uint32_t* &pPPosting, int32_t& posBufLength, int32_t minBufLength) const
    {
        const int32_t upperBound = UncompressedOutBufferUpperbound(minBufLength);
        if(posBufLength < upperBound)
            growPosBuffer(pPPosting, posBufLength, upperBound);
    }
};

}

NS_IZENELIB_IR_END


#endif

