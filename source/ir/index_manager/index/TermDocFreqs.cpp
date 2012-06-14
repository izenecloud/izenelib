#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/RTPostingReader.h>
#include <ir/index_manager/index/CompressParameters.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

TermDocFreqs::TermDocFreqs()
        :pPosting_(NULL)
        ,pPostingBuffer_(0)
        ,nBufferSize_(0)
        ,nMaxDocCount_(0)
        ,nFreqStart_(0)
        ,nTotalDecodedCount_(0)
        ,nCurDecodedCount_(0)
        ,nCurrentPosting_(-1)
        ,ownPosting_(true)
{
}

TermDocFreqs::TermDocFreqs(
    PostingReader* pPosting,
    const TermInfo& ti,
    bool ownPosting)
{
    reset(pPosting,ti,ownPosting)	;
}

TermDocFreqs::~TermDocFreqs()
{
    close();
    if (pPostingBuffer_)
    {
        delete[] pPostingBuffer_;
        pPostingBuffer_ = NULL;
    }
    nBufferSize_ = 0;
}

void TermDocFreqs::reset(
    PostingReader * pPosting,
    const TermInfo& ti,
    bool ownPosting)
{
    termInfo_ = ti;
    pPosting_ = pPosting;
    pPostingBuffer_ = NULL;
    nBufferSize_ = 0;
    nMaxDocCount_ = 0;
    nFreqStart_ = 0;
    nTotalDecodedCount_ = 0;
    nCurDecodedCount_ = 0;
    nCurrentPosting_ = -1;
    ownPosting_ = ownPosting;
}

docid_t TermDocFreqs::skipTo(docid_t target)
{
    int32_t start,end;
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
                return pPosting_->DecodeTo(target,pPostingBuffer_,
                               nBufferSize_,nMaxDocCount_,nCurDecodedCount_,nCurrentPosting_);
            }
       }
        start = nCurrentPosting_;
        end = start + (target - pPostingBuffer_[nCurrentPosting_]);
        if(end == start)
        {
            return target;
        }
        else if(end < start)
        {
            return pPostingBuffer_[nCurrentPosting_];
        }
        if ( end > (nCurDecodedCount_ - 1) )
            end = nCurDecodedCount_ - 1;
        docid_t nRetDocId;
        nCurrentPosting_ = bsearch(pPostingBuffer_,start,end,target,nRetDocId);///binary search in decoded buffer
        if(nRetDocId >= target)
            return nRetDocId;
        nCurrentPosting_ = nCurDecodedCount_;///buffer is over
    }
}

int32_t TermDocFreqs::bsearch(
    docid_t docs[],
    int32_t start,
    int32_t end,
    docid_t key,
    docid_t& keyFound)
{
    int32_t k;
    int32_t nk = end;
    keyFound = docs[end];
    while (start<=end)
    {
        k = (start + end)/2;
        if(key == docs[k])
        {
            keyFound = key;
            return k;
        }
        if(key < docs[k])
        {
            end = k - 1;
            if(k >= start)
            {
                keyFound = docs[k];
                nk =k;
            }
        }
        else
        {
            start = k + 1;
            if( (start <= end) && (docs[start] > key) )
            {
                keyFound = docs[start];
                nk = start;
            }
        }
    }
    return nk;
}

void TermDocFreqs::close()
{
    if (pPosting_)
    {
        if(ownPosting_)
            delete pPosting_;
        pPosting_ = NULL;
    }
}

void TermDocFreqs::createBuffer()
{
    if (pPostingBuffer_ == NULL)
    {
        nMaxDocCount_ = CHUNK_SIZE;
        nFreqStart_ = UncompressedOutBufferUpperbound(CHUNK_SIZE);
        nBufferSize_ = nFreqStart_ << 1;
        pPostingBuffer_ = new uint32_t[nBufferSize_];
        memset(pPostingBuffer_, 0, nBufferSize_*sizeof(uint32_t));
    }
}

}

NS_IZENELIB_IR_END
