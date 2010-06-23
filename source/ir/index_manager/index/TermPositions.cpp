#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/InputDescriptor.h>

#include <util/profiler/ProfilerGroup.h>
//#define SF1_TIME_CHECK

using namespace izenelib::ir::indexmanager;

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

TermPositions::TermPositions(TermReader* pReader,InputDescriptor* pInputDescriptor,const TermInfo& ti)
        :TermDocFreqs(pReader,pInputDescriptor,ti)
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

TermPositions::TermPositions(TermReader* pReader,Posting* pPosting,const TermInfo& ti)
        :TermDocFreqs(pReader,pPosting,ti)
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
        ,bOwnPPostingBuffer_(false)
{}

TermPositions::TermPositions(Posting* pPosting, const TermInfo& ti)
        :TermDocFreqs(pPosting,ti)
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
        ,bOwnPPostingBuffer_(false)
{}

TermPositions::~TermPositions()
{
    TermDocFreqs::close();

    if (pPPostingBuffer_)
        delete[] pPPostingBuffer_;
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
            //if(termInfo_.docFreq_ < 4096)
            if(Posting::skipInterval_ == 0)
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
CREATE_SCOPED_PROFILER ( getpositions, "IndexManager", "nextPosition: get positions");

    if (nCurrentPPostingWithinDoc_ >= nCurDecodedPCountWithinDoc_)
    {
        if (!decodePositions())
            return BAD_POSITION;
        return pPPostingBufferWithinDoc_[nCurrentPPostingWithinDoc_++];
    }

    return pPPostingBufferWithinDoc_[nCurrentPPostingWithinDoc_++];
}

int32_t TermPositions::nextPositions(loc_t*& positions)
{
    if (nCurrentPPostingWithinDoc_ >= nCurDecodedPCountWithinDoc_)
    {
        if (!decodePositions())
            return -1;
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
        pPPostingBuffer_ = new uint32_t[bufSize];
        memset(pPPostingBuffer_, 0, bufSize*sizeof(uint32_t));
    }
}

