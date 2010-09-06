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

    TermPositions(TermReader* pReader,InputDescriptor* pInputDescriptor,const TermInfo& ti,int skipInterval, int maxSkipLevel);

    TermPositions(TermReader* pReader,PostingReader* pPosting,const TermInfo& ti);

    TermPositions(PostingReader* pPosting,const TermInfo& ti);

    virtual ~TermPositions(void);
public:
    /**
     * get document id
     * @return document id
     */
    virtual docid_t doc();

    virtual count_t freq();
    /**
     * move to the next documents block
     */
    virtual bool	next();
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

    virtual void	close();
    /**
    * iterate to next position
    */
    virtual loc_t	nextPosition();
    /**
    * iterate to next block of positions
    */
    virtual int32_t nextPositions(loc_t*& positions);
private:
    /** decode positions */
    inline bool decodePositions();

    /** skip some positions */
    inline void skipPositions(int32_t nSkips);

    /** create buffer for decoding */
    void createBuffer();

    void resetDecodingState();
	
private:
    uint32_t* pPPostingBuffer_;				///buffer for position posting

    size_t nPBufferSize_;					///size of buffer

    int32_t nTotalDecodedPCountWithinDoc_;

    int32_t nCurDecodedPCountWithinDoc_;

    int32_t nCurrentPPostingWithinDoc_;

    size_t nTotalDecodedPCount_;

    size_t nCurrentPPosting_;

    int32_t nLastUnDecodedPCount_;

    uint32_t* pPPostingBufferWithinDoc_;

    int32_t nPPostingCountWithinDoc_;		///total position posting count

    int32_t nLastPosting_;

    bool bOwnPPostingBuffer_;			///we own the buffer or not

    static const size_t DEFAULT_BUFFERSIZE = 163840;
};

//////////////////////////////////////////////////////////////////////////
///inline functions
inline void TermPositions::skipPositions(int32_t nSkips)
{
    if (nSkips <= 0)
        return;

    if ((size_t)nSkips <= (nTotalDecodedPCount_ - nCurrentPPosting_))
        nCurrentPPosting_ += nSkips;
    else
    {
        pPosting_->decodeNextPositions(NULL,(int32_t)(nSkips - (nTotalDecodedPCount_ - nCurrentPPosting_)));
        nCurrentPPosting_ = nTotalDecodedPCount_;
        nLastPosting_ = -1;
    }
}

inline bool TermPositions::decodePositions()
{
    if (nTotalDecodedPCountWithinDoc_ >= nPPostingCountWithinDoc_)
    {
        ///reach to the end
        return false;
    }
    if (!pPPostingBuffer_)
        createBuffer();
    if ((size_t)nCurrentPPosting_ >= nTotalDecodedPCount_)///decode position posting to buffer
    {
        uint32_t* pPPostingBuffer = pPPostingBuffer_;
        bool bEnd = false;
        if (nLastPosting_ == -1)
            nLastPosting_ = nCurrentPosting_;
        nTotalDecodedPCount_ = 0;
        if (nLastUnDecodedPCount_ > 0)
        {
            if ((size_t)nLastUnDecodedPCount_ > nPBufferSize_)
            {
                nTotalDecodedPCount_ = nPBufferSize_;
                nLastUnDecodedPCount_ -= (int32_t)nPBufferSize_;
                bEnd = true;
            }
            else
            {
                nTotalDecodedPCount_ = nLastUnDecodedPCount_;
                nLastUnDecodedPCount_ = 0;
                nLastPosting_++;
            }
            bool ret = pPosting_->decodeNextPositions(pPPostingBuffer,(int32_t)nTotalDecodedPCount_);
            if(!ret) return false;
            pPPostingBuffer += nTotalDecodedPCount_;
            if (nLastUnDecodedPCount_ == 0)
                pPosting_->resetPosition();
        }
        if (!bEnd)
        {
            int32_t nFreqs;
            for (nFreqs = nLastPosting_; nFreqs < nCurDecodedCount_;nFreqs++)
            {
                nTotalDecodedPCount_ += pPostingBuffer_[nFreqStart_ + nFreqs];
                if (nTotalDecodedPCount_ > nPBufferSize_)
                {
                    nTotalDecodedPCount_ -= pPostingBuffer_[nFreqStart_+ nFreqs];
                    break;
                }
            }
            if ((nFreqs - nLastPosting_) > 0)
            {
                bool ret = pPosting_->decodeNextPositions(pPPostingBuffer,&(pPostingBuffer_[nFreqStart_ + nLastPosting_]),nFreqs - nLastPosting_);
                if(!ret) return false;
                nLastPosting_ = nFreqs;
            }
            else if (nTotalDecodedPCount_ == 0)
            {
                nTotalDecodedPCount_ = nPBufferSize_;
                bool ret = pPosting_->decodeNextPositions(pPPostingBuffer,(int32_t)nTotalDecodedPCount_);
                if(!ret) return false;
                nLastUnDecodedPCount_ = pPostingBuffer_[nFreqStart_ + nLastPosting_] - (int32_t)nTotalDecodedPCount_;
            }
        }
        nCurrentPPosting_ = 0;
        if (nLastPosting_ >= nCurDecodedCount_)
            nLastPosting_ = -1;
    }

    nCurrentPPostingWithinDoc_ = 0;

    int32_t nNeedDecode = nPPostingCountWithinDoc_ - nTotalDecodedPCountWithinDoc_;
    if ((size_t)nNeedDecode > (nTotalDecodedPCount_ - nCurrentPPosting_))
        nNeedDecode = (int32_t)(nTotalDecodedPCount_ - nCurrentPPosting_);

    nCurDecodedPCountWithinDoc_ = nNeedDecode;
    nTotalDecodedPCountWithinDoc_ += nNeedDecode;
    pPPostingBufferWithinDoc_ = pPPostingBuffer_ + nCurrentPPosting_;
    nCurrentPPosting_ += nNeedDecode;

    return true;
}

}

NS_IZENELIB_IR_END

#endif
