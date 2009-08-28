#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/InputDescriptor.h>


using namespace izenelib::ir::indexmanager;

TermPositions::TermPositions(void)
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
{
}

TermPositions::TermPositions(TermReader* pReader,InputDescriptor* pInputDescriptor,TermInfo& ti)
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
{
}
TermPositions::TermPositions(TermReader* pReader,Posting* pPosting,TermInfo& ti)
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
{
}
TermPositions::~TermPositions(void)
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
        nPPostingCountWithinDoc_ = freq()*2;//*2 because the offset of a document is a pair
        nTotalDecodedPCountWithinDoc_ = 0;
        nCurDecodedPCountWithinDoc_ = 0;
        nCurrentPPostingWithinDoc_ = 0;
        pPosting->resetPosition();
        return true;
    }
    return false;
}

count_t TermPositions::next(docid_t*& docs, count_t*& freqs)
{
    throw UnsupportedOperationException("don't support TermPositions::next(docid_t*& docs, count_t*& freqs),please use TermDocFreqs.");
}

void TermPositions::close()
{
    TermDocFreqs::close();
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
    }
}

