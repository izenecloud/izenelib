#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/InputDescriptor.h>

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
{}

TermPositions::TermPositions(PostingReader* pPosting,const TermInfo& ti,bool ownPosting)
        :TermDocFreqs(pPosting,ti,ownPosting)
        ,pPPostingBuffer_(NULL)
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
{}

TermPositions::~TermPositions()
{
    TermDocFreqs::close();

    if (pPPostingBuffer_)
        free(pPPostingBuffer_);
    pPPostingBuffer_ = NULL;
    nPBufferSize_ = 0;
    pPPostingBufferWithinDoc_ = NULL;
}

docid_t TermPositions::doc()
{
    return TermDocFreqs::doc();
}

count_t TermPositions::freq()
{
    return TermDocFreqs::freq();
}

bool TermPositions::next()
{
    if (TermDocFreqs::next())
    {
        nCurDecodedPCountWithinDoc_ = freq();
        nCurrentPPostingWithinDoc_ = 0;
        return true;
    }
    return false;
/*
    ///skip the previous document's positions
    skipPositions(nPPostingCountWithinDoc_ - nTotalDecodedPCountWithinDoc_);

    if (TermDocFreqs::next())
    {
        nPPostingCountWithinDoc_ = freq();
        nTotalDecodedPCountWithinDoc_ = 0;
        nCurDecodedPCountWithinDoc_ = 0;
        nCurrentPPostingWithinDoc_ = 0;
        pPosting_->resetPosition();
        return true;
    }
    return false;
*/
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
            if((termInfo_.docFreq_ < 4096)||(skipInterval_ == 0))
            {
                if(!TermDocFreqs::decode())
                    return BAD_DOCID;
            }
            else
            {
                if(!pPostingBuffer_)
                    TermDocFreqs::createBuffer();
                nCurrentPosting_ = 0;
                nCurDecodedCount_ = 1;
                pPostingBuffer_[0] = pPosting_->decodeTo(target);
                pPostingBuffer_[nFreqStart_] = pPosting_->getCurTF();
                resetDecodingState();
                return pPostingBuffer_[0];
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
    pPosting_->resetPosition();
}

void TermPositions::close()
{
    TermDocFreqs::close();
}

loc_t TermPositions::nextPosition()
{
    if (nCurrentPPostingWithinDoc_ >= nCurDecodedPCountWithinDoc_)
    {
        return BAD_POSITION;
/*    
        if (!decodePositions())
            return BAD_POSITION;
        return pPPostingBufferWithinDoc_[nCurrentPPostingWithinDoc_++];
*/
    }

    return pPPostingBufferWithinDoc_[nCurrentPPostingWithinDoc_++];
}

int32_t TermPositions::nextPositions(loc_t*& positions)
{
    if (nCurrentPPostingWithinDoc_ >= nCurDecodedPCountWithinDoc_)
    {
        return BAD_POSITION;
/*    
        if (!decodePositions())
            return -1;
*/
    }

    positions = pPPostingBufferWithinDoc_;
    return nCurDecodedPCountWithinDoc_;
}

void TermPositions::createBuffer()
{
    size_t bufSize = TermPositions::DEFAULT_BUFFERSIZE;
    if ((int64_t)bufSize > getCTF())
        bufSize = (size_t)getCTF();
    if (pPPostingBuffer_ == NULL)
    {
        nPBufferSize_ = bufSize;
        pPPostingBuffer_ = (uint32_t*)malloc(bufSize*sizeof(uint32_t));		
        memset(pPPostingBuffer_, 0, bufSize*sizeof(uint32_t));
    }
}

bool TermPositions::decode()
{
    count_t df = termInfo_.docFreq();
    if ((count_t)nTotalDecodedCount_ == df)
    {
        return false;
    }

    if (!pPostingBuffer_)
        createBuffer();

    nCurrentPosting_ = 0;
    nCurDecodedCount_ = pPosting_->decodeNext(pPostingBuffer_,nBufferSize_, pPPostingBuffer_, nPBufferSize_);
    if (nCurDecodedCount_ <= 0)
        return false;
    nTotalDecodedCount_ += nCurDecodedCount_;

    return true;
}

}

NS_IZENELIB_IR_END

