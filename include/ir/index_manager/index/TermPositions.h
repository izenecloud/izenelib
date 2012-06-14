/**
* @file        TermPositions.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   Iterate the index of position posting (*.pop)
*/
#ifndef TermPositions_H
#define TermPositions_H

#include <ir/index_manager/index/TermDocFreqs.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* Iterate the index of position posting (*.pop)
* @note Iterating position posting also require the term documentfrequency postings to be iterated, therefore
* TermPositions also inherits from TermDocFreqs.
* TermPositions iterates index in a certain barrel only, multi barrels should use MultiTermPositions
*/
class TermPositions: public TermDocFreqs
{
public:
    TermPositions();

    TermPositions(
        PostingReader* pPosting,
        const TermInfo& ti,
        bool ownPosting = true);

    virtual ~TermPositions();
public:
    virtual void reset(
        PostingReader * pPosting,
        const TermInfo& ti,
        bool ownPosting = true);
    /**
     * get document id
     * @return document id
     */
    virtual docid_t doc();

    virtual count_t freq();
    /**
     * move to the next documents block
     */
    virtual bool next();
    /*
    * move to the first document whose id is greater than or equal to target
    */
    virtual docid_t skipTo(docid_t docId);
    /**
     * get document frequency
     * @return frequency
     */
    virtual freq_t docFreq()
    {
        return TermDocFreqs::docFreq();
    };

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    virtual int64_t getCTF()
    {
        return TermDocFreqs::getCTF();
    };

    /**
    * iterate to next position
    */
    virtual loc_t nextPosition();

    void setUseFixedPosBuffer(bool useFixedBuf)
    {
        fixed_pos_buffer_ = useFixedBuf;
    }
protected:
    virtual bool decode();

    /** decode positions */
    inline bool decodePositions();

    bool decodePositionsUsingFixedBuf();

    bool decodePositionsUsingUnFixedBuf();

    /** skip some positions */
    inline void skipPositions(int32_t nSkips);

    /** create buffer for decoding */
    void createBuffer();

    void resetDecodingState();

private:
    uint32_t* pPPostingBuffer_;				///buffer for position posting

    int32_t nPBufferSize_;					///size of buffer

    int32_t nTotalDecodedPCountWithinDoc_;   ///number of decoded positions for one doc

    int32_t nCurDecodedPCountWithinDoc_;

    int32_t nCurrentPPostingWithinDoc_;

    int32_t nTotalDecodedPCount_;

    int32_t nCurrentPPosting_;

    int32_t nLastUnDecodedPCount_;

    uint32_t* pPPostingBufferWithinDoc_;

    int32_t nPPostingCountWithinDoc_;		///total position posting count   TF

    int32_t nLastPosting_;

    bool is_last_skipto_; /// last operation is skipto

    /**
     * whether using fixed position buffer to decode term positions,
     * this flag is not used as RTDiskPostingReader::DecodeNextPositions()
     * has bugs when this flag is true.
     */
    bool fixed_pos_buffer_; 
};

//////////////////////////////////////////////////////////////////////////
///inline functions
inline docid_t TermPositions::doc()
{
    return TermDocFreqs::doc();
}

inline count_t TermPositions::freq()
{
    return TermDocFreqs::freq();
}

inline void TermPositions::skipPositions(int32_t nSkips)
{
    if (nSkips <= 0)
        return;
    if(nTotalDecodedPCount_ == 0)
    {
        pPosting_->DecodeNextPositions(NULL,nSkips);
        nCurrentPPosting_ = nTotalDecodedPCount_;
    }
    else if (nSkips <= (nTotalDecodedPCount_ - nCurrentPPosting_))
        nCurrentPPosting_ += nSkips;
    else
    {
        pPosting_->DecodeNextPositions(NULL,(int32_t)(nSkips - (nTotalDecodedPCount_ - nCurrentPPosting_)));
        nCurrentPPosting_ = nTotalDecodedPCount_;
        nLastPosting_ = -1;
    }
}

inline bool TermPositions::decodePositions()
{
    /*if(fixed_pos_buffer_) return decodePositionsUsingFixedBuf();*/
    return decodePositionsUsingUnFixedBuf();
}

}

NS_IZENELIB_IR_END

#endif
