/**
* @file        PostingRead.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief posting reader
*/

#ifndef POSTING_READER_H
#define POSTING_READER_H

#include "types.h"
#include <ir/index_manager/utility/system.h>
#include <cassert>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class Bitset;
struct PostingReader
{
public:
    virtual ~PostingReader(){}

    /**
     * Continue to decode the posting of doc and freq.
     * @param pPosting the address to store the decoded doc and freq,
     *                 the docs are stored from pPosting[0],
     *                 the freqs are stored from pPosting[length/2]
     * @param length the length of @p pPosting
     * @param nMaxDocs the maximum doc count allowed to decode
     * @return decoded doc count
     * @pre 0 < @p nMaxDocs <= @p length/2, as some decompression methods such as S9 and S16 need extra space to store decoded data
     * @post return value <= @p nMaxDocs
     */
    virtual int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs) = 0;

    /**
     * Continue to decode the posting of doc, freq and position.
     * @param pPosting the address to store the decoded doc and freq,
     *                 the docs are stored from pPosting[0],
     *                 the freqs are stored from pPosting[length/2]
     * @param length the length of @p pPosting
     * @param nMaxDocs the maximum doc count allowed to decode
     * @param pPPosting the address to store the decoded position
     * @param posBufLength the length of @p pPPosting
     * @param posLength store decoded positions count on return
     * @return decoded doc count
     * @pre 0 < @p nMaxDocs <= @p length/2, as some decompression methods such as S9 and S16 need extra space to store decoded data
     * @post return value <= @p nMaxDocs
     */
    virtual int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs,
        uint32_t* &pPPosting,
        int32_t& posBufLength,
        int32_t& posLength) = 0;

    virtual bool DecodeNextPositions(
        uint32_t* pPosting,
        int32_t length) = 0;

    virtual bool DecodeNextPositions(
        uint32_t* &pPosting,
        int32_t& posBufLength,
        int32_t decodeLength,
        int32_t& nCurrentPPosting) = 0;

    virtual bool DecodeNextPositions(
        uint32_t* &pPosting,
        int32_t& posBufLength,
        uint32_t* pFreqs,
        int32_t nFreqs,
        int32_t& nCurrentPPosting) = 0;

    virtual void ResetPosition() = 0;

    /**
     * Decode the posting of doc and freq to @p target docID
     * @param target target docID
     * @param pPosting the address to store the decoded doc and freq
     * @param length the length of @p pPosting
     * @param nMaxDocs the maximum doc count allowed to decode
     * @param decodedCount store decoded doc count on return
     * @param nCurrentPosting store the start decoded position in @p pPosting on return,
     *                        the docs are stored from pPosting[nCurrentPosting],
     *                        the freqs are stored from pPosting[length/2 + nCurrentPosting]
     * @return the value of pPosting[nCurrentPosting] if it is >= @p target, otherwise @c BAD_DOCID is returned
     * @pre 0 < @p nMaxDocs <= @p length/2, as some decompression methods such as S9 and S16 need extra space to store decoded data
     * @post @p decodedCount <= @p nMaxDocs
     */
    virtual docid_t DecodeTo(
        docid_t target,
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs,
        int32_t& decodedCount,
        int32_t& nCurrentPosting) = 0;

    virtual count_t docFreq()const = 0;

    virtual int64_t getCTF()const = 0;

    virtual void setFilter(Bitset* pFilter) = 0;

protected:
    /**
     * Grow the position buffer size until @p minBufLength is reached.
     * @param pPPosting reference to position buffer, it stores the grown buffer on return
     * @param posBufLength position buffer length, it stores grown buffer size on return
     * @param minBufLength minimum buffer length, @p posBufLength would not be less than this value on return
     */
    static void GrowPosBuffer(
        uint32_t* &pPPosting,
        int32_t& posBufLength,
        int32_t minBufLength)
    {
        assert(posBufLength < minBufLength);

        while(posBufLength < minBufLength)
            posBufLength <<= 1;

        uint32_t* p = (uint32_t*)realloc(pPPosting, posBufLength * sizeof(uint32_t));
        if(p) pPPosting = p;
        else SF1V5_THROW(ERROR_OUTOFMEM,"PostingReader:GrowPosBuffer() : ReAllocate memory failed.");

        assert(posBufLength >= minBufLength);
    }

    /**
     * Grow the position buffer size @p posBufLength until an upper bound of @p minBufLength is reached.
     * @param pPPosting reference to position buffer, it stores the grown buffer on return
     * @param posBufLength position buffer length, it stores grown buffer size on return
     * @param minBufLength minimum buffer length, @p posBufLength would not be less than the upper bound of this value on return
     * @note the upper bound is met as S9 and S16 decoding need at least 28 extra bytes for decompression.
     */
    static void EnsurePosBufferUpperBound(
        uint32_t* &pPPosting,
        int32_t& posBufLength,
        int32_t minBufLength);
};

}

NS_IZENELIB_IR_END


#endif
