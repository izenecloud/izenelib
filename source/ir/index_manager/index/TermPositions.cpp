#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/CompressParameters.h>

#include <algorithm>

namespace
{
/** maximum value for position buffer size. */
const int32_t MAX_POS_BUFFER_SIZE = 163840;
}

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

TermPositions::TermPositions()
    :pPPostingBuffer_(NULL)
    ,nPBufferSize_(0)
    ,nTotalDecodedPCountWithinDoc_(0)
    ,nCurDecodedPCountWithinDoc_(0)
    ,nCurrentPPostingWithinDoc_(0)
    ,nTotalDecodedPCount_(0)
    ,nCurrentPPosting_(0)
    ,nLastUnDecodedPCount_(0)
    ,pPPostingBufferWithinDoc_(NULL)
    ,nPPostingCountWithinDoc_(0)
    ,nLastPosting_(-1)
    ,is_last_skipto_(false)
    ,fixed_pos_buffer_(false)
{}

TermPositions::TermPositions(
    PostingReader* pPosting,
    const TermInfo& ti,
    bool ownPosting)
{
    reset(pPosting,ti,ownPosting)	;
}

TermPositions::~TermPositions()
{
    if (pPPostingBuffer_)
        free(pPPostingBuffer_);
    pPPostingBuffer_ = NULL;
    nPBufferSize_ = 0;
    pPPostingBufferWithinDoc_ = NULL;
}

void TermPositions::reset(
    PostingReader * pPosting,
    const TermInfo& ti,
    bool ownPosting)
{
    TermDocFreqs::reset(pPosting,ti,ownPosting);
    pPPostingBuffer_ = NULL;
    nPBufferSize_ = 0;
    nTotalDecodedPCountWithinDoc_ = 0;
    nCurDecodedPCountWithinDoc_ = 0;
    nCurrentPPostingWithinDoc_ = 0;
    nTotalDecodedPCount_ = 0;
    nCurrentPPosting_ = 0;
    nLastUnDecodedPCount_ = 0;
    pPPostingBufferWithinDoc_ = NULL;
    nPPostingCountWithinDoc_ = 0;
    nLastPosting_ = -1;
    is_last_skipto_ = false;
    fixed_pos_buffer_ = false;
}

bool TermPositions::next()
{
    skipPositions(nPPostingCountWithinDoc_ - nTotalDecodedPCountWithinDoc_);
    is_last_skipto_ = false;

    if (TermDocFreqs::next())
    {
        nPPostingCountWithinDoc_ = freq();
        nTotalDecodedPCountWithinDoc_ = 0;
        nCurDecodedPCountWithinDoc_ = 0;
        nCurrentPPostingWithinDoc_ = 0;
        //pPPostingBufferWithinDoc_ = pPPostingBuffer_ + nCurrentPPosting_;
        pPosting_->ResetPosition();
        return true;
    }
    return false;
}

docid_t TermPositions::skipTo(docid_t target)
{

    int32_t start,end,skip;
    int32_t i = 0;
    docid_t foundDocId = -1;
    while (true)
    {
        if((nCurrentPosting_ == -1) || (nCurrentPosting_ >= nCurDecodedCount_) )
        {
            if(termInfo_.docFreq_ < 4096)
            {
                if(!decode())
                    return BAD_DOCID;
            }
            else
            {
                if(!pPostingBuffer_)
                    createBuffer();
                docid_t ret = pPosting_->DecodeTo(target,pPostingBuffer_,nBufferSize_,
                                                   nMaxDocCount_,nCurDecodedCount_,nCurrentPosting_);
                resetDecodingState();
                nCurrentPPosting_ = 0;
                nTotalDecodedPCount_ = 0;
                is_last_skipto_ = true;
                return ret;
            }
        }
        start = nCurrentPosting_;
        end = start + (target - pPostingBuffer_[nCurrentPosting_]);
        if(end == start)
        {
            resetDecodingState();
            return target;
        }
        else if(end < start)
        {
            resetDecodingState();
            return pPostingBuffer_[nCurrentPosting_];
        }
        if ( end > (nCurDecodedCount_ - 1) )
            end = nCurDecodedCount_ - 1;

        nCurrentPosting_ = bsearch(pPostingBuffer_,start,end,target,foundDocId);///binary search in decoded buffer
        ///skip positions
        skip = (pPostingBuffer_[nFreqStart_ + start] - nTotalDecodedPCountWithinDoc_);
        for (i = start + 1;i < nCurrentPosting_;i++)
        {
            skip += pPostingBuffer_[nFreqStart_ + i];
        }
        skipPositions(skip);

        if(foundDocId >= target)
        {
            resetDecodingState();
            return foundDocId;
        }

        if(start != nCurrentPosting_)
            skipPositions(pPostingBuffer_[nFreqStart_ + nCurrentPosting_]);
        nTotalDecodedPCountWithinDoc_ = 0;
        nCurDecodedPCountWithinDoc_ = 0;
        nCurrentPPostingWithinDoc_ = 0;
        nCurrentPosting_ = nCurDecodedCount_;///buffer is over
    }
}

void TermPositions::resetDecodingState()
{
    nPPostingCountWithinDoc_ = pPostingBuffer_[nFreqStart_ + nCurrentPosting_];
    nCurDecodedPCountWithinDoc_ = 0;
    nTotalDecodedPCountWithinDoc_ = 0;
    nCurrentPPostingWithinDoc_ = 0;
    pPosting_->ResetPosition();
}

loc_t TermPositions::nextPosition()
{
    if (nCurrentPPostingWithinDoc_ >= nCurDecodedPCountWithinDoc_)
    {
        if (!decodePositions())
            return BAD_POSITION;
        return pPPostingBufferWithinDoc_[nCurrentPPostingWithinDoc_++];
    }

    return pPPostingBufferWithinDoc_[nCurrentPPostingWithinDoc_++];
}

bool TermPositions::decodePositionsUsingUnFixedBuf()
{
    if (nTotalDecodedPCountWithinDoc_ >= nPPostingCountWithinDoc_)
    {
        ///reach to the end
        return false;
    }
    if (!pPPostingBuffer_)
        createBuffer();
    if (nCurrentPPosting_ >= nTotalDecodedPCount_)
    {
        ///decode position posting to buffer
        if(!is_last_skipto_)
        {
            return false;
        }
       else
	{
            nTotalDecodedPCount_ = 0;
            int32_t nFreqs;
            for (nFreqs = 0; nFreqs < nCurDecodedCount_; nFreqs++)
            {
                nTotalDecodedPCount_ += pPostingBuffer_[nFreqStart_ + nFreqs];
            }

            //pPosting_->DecodeNextPositions(pPPostingBuffer_, nPBufferSize_, NULL, 0, nCurrentPPosting_);
            pPosting_->DecodeNextPositions(pPPostingBuffer_,nPBufferSize_, 
                                             nTotalDecodedPCount_, nCurrentPPosting_);
	}
		
    }

    nCurrentPPostingWithinDoc_ = 0;

    nCurDecodedPCountWithinDoc_ = nPPostingCountWithinDoc_;
    nTotalDecodedPCountWithinDoc_ += nPPostingCountWithinDoc_;
    pPPostingBufferWithinDoc_ = pPPostingBuffer_ + nCurrentPPosting_;
    nCurrentPPosting_ += nPPostingCountWithinDoc_;

    return true;
}

bool TermPositions::decodePositionsUsingFixedBuf()
{
    if (nTotalDecodedPCountWithinDoc_ >= nPPostingCountWithinDoc_)
    {
        ///reach to the end
        return false;
    }
    if (!pPPostingBuffer_)
        createBuffer();
    if (nCurrentPPosting_ >= nTotalDecodedPCount_)
    {
        ///decode position posting to buffer
        uint32_t* pPPostingBuffer = pPPostingBuffer_;
        bool bEnd = false;
        if (nLastPosting_ == -1)
            nLastPosting_ = nCurrentPosting_;
        nTotalDecodedPCount_ = 0;

        if (nLastUnDecodedPCount_ > 0)
        {
            ///there are still some positions of current document have not decoded
            if (nLastUnDecodedPCount_ > nPBufferSize_)
            {
                nTotalDecodedPCount_ = nPBufferSize_;
                nLastUnDecodedPCount_ -= nPBufferSize_;
                bEnd = true;
            }
            else
            {
                nTotalDecodedPCount_ = nLastUnDecodedPCount_;
                nLastUnDecodedPCount_ = 0;
                nLastPosting_++;
            }
            bool ret = pPosting_->DecodeNextPositions(pPPostingBuffer,nTotalDecodedPCount_);
            if(!ret) return false;
            pPPostingBuffer += nTotalDecodedPCount_;
            if (nLastUnDecodedPCount_ == 0)
                pPosting_->ResetPosition();
        }
        if (!bEnd)
        {
            int32_t nFreqs;
            for (nFreqs = nLastPosting_; nFreqs < nCurDecodedCount_; nFreqs++)
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
                bool ret = pPosting_->DecodeNextPositions(pPPostingBuffer, 
                                    nPBufferSize_ ,&(pPostingBuffer_[nFreqStart_ + nLastPosting_]),
                                    nFreqs - nLastPosting_,nCurrentPPosting_);
                if(!ret) return false;
                nLastPosting_ = nFreqs;
            }
            else if (nTotalDecodedPCount_ == 0)
            {
                nTotalDecodedPCount_ = nPBufferSize_;
                bool ret = pPosting_->DecodeNextPositions(pPPostingBuffer,nTotalDecodedPCount_);
                if(!ret) return false;
                nLastUnDecodedPCount_ = pPostingBuffer_[nFreqStart_ + nLastPosting_] - nTotalDecodedPCount_;
            }
        }
        nCurrentPPosting_ = 0;
        if (nLastPosting_ >= nCurDecodedCount_)
            nLastPosting_ = -1;
    }

    nCurrentPPostingWithinDoc_ = 0;

    int32_t nNeedDecode = nPPostingCountWithinDoc_ - nTotalDecodedPCountWithinDoc_;
    if (nNeedDecode > (nTotalDecodedPCount_ - nCurrentPPosting_))
        nNeedDecode = (int32_t)(nTotalDecodedPCount_ - nCurrentPPosting_);

    nCurDecodedPCountWithinDoc_ = nNeedDecode;
    nTotalDecodedPCountWithinDoc_ += nNeedDecode;
    pPPostingBufferWithinDoc_ = pPPostingBuffer_ + nCurrentPPosting_;
    nCurrentPPosting_ += nNeedDecode;

    return true;
}

void TermPositions::createBuffer()
{
    TermDocFreqs::createBuffer();

    if (pPPostingBuffer_ == NULL)
    {
        // as some block decompression methods such as s16_compressor::s16_decode()
        // need extra 28 bytes to store decompressed data,
        // set an upper bound for buffer size
        nPBufferSize_ = UncompressedOutBufferUpperbound(getCTF());
        // set the maximum position buffer size
        if(nPBufferSize_ > MAX_POS_BUFFER_SIZE)
            nPBufferSize_ = MAX_POS_BUFFER_SIZE;

        pPPostingBuffer_ = (uint32_t*)malloc(nPBufferSize_*sizeof(uint32_t));		
        memset(pPPostingBuffer_, 0, nPBufferSize_*sizeof(uint32_t));
    }
}

bool TermPositions::decode()
{
    //if(fixed_pos_buffer_) return TermDocFreqs::decode();
    count_t df = termInfo_.docFreq();
    if ((count_t)nTotalDecodedCount_ == df)
    {
        return false;
    }

    if (!pPostingBuffer_)
        createBuffer();

    nCurrentPosting_ = 0;
    nCurrentPPosting_ = 0;
    nCurDecodedCount_ = pPosting_->DecodeNext(pPostingBuffer_, 
                    nBufferSize_, nMaxDocCount_, pPPostingBuffer_, 
                    nPBufferSize_, nTotalDecodedPCount_);
    if (nCurDecodedCount_ <= 0)
        return false;
    nTotalDecodedCount_ += nCurDecodedCount_;

    return true;
}

}

NS_IZENELIB_IR_END

