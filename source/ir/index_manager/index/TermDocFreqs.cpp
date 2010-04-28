#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Term.h>


using namespace izenelib::ir::indexmanager;

TermDocFreqs::TermDocFreqs(void)
        :pPosting_(NULL)
        ,pPostingBuffer_(0)
        ,nBufferSize_(0)
        ,nFreqStart_(0)
        ,nTotalDecodedCount_(0)
        ,nCurDecodedCount_(0)
        ,nCurrentPosting_(-1)
        ,pTermReader_(NULL)
        ,pInputDescriptor_(NULL)
        ,ownPosting_(true)
{
}

TermDocFreqs::TermDocFreqs(TermReader* pReader, InputDescriptor* pInputDescriptor, TermInfo& ti)
        :pPostingBuffer_(NULL)
        ,nBufferSize_(0)
        ,nFreqStart_(0)
        ,nTotalDecodedCount_(0)
        ,nCurDecodedCount_(0)
        ,nCurrentPosting_(-1)
        ,pTermReader_(pReader)
        ,pInputDescriptor_(pInputDescriptor)
        ,ownPosting_(true)
{
    termInfo_.set(ti.docFreq(),ti.docPointer());
    //pInputDescriptor_->getDPostingInput()->seek(ti.docPointer());
    pPosting_ = new OnDiskPosting(pInputDescriptor_,ti.docPointer());
}

TermDocFreqs::TermDocFreqs(TermReader* pReader, Posting* pPosting, TermInfo& ti)
        :pPosting_(pPosting->clone())
        ,pPostingBuffer_(NULL)
        ,nBufferSize_(0)
        ,nFreqStart_(0)
        ,nTotalDecodedCount_(0)
        ,nCurDecodedCount_(0)
        ,nCurrentPosting_(-1)
        ,pTermReader_(pReader)
        ,pInputDescriptor_(NULL)
        ,ownPosting_(true)
{
    termInfo_.set(ti.docFreq(),ti.docPointer());
}

TermDocFreqs::TermDocFreqs(Posting* pPosting)
        :pPosting_(pPosting)
        ,pPostingBuffer_(NULL)
        ,nBufferSize_(0)
        ,nFreqStart_(0)
        ,nTotalDecodedCount_(0)
        ,nCurDecodedCount_(0)
        ,nCurrentPosting_(-1)
        ,pTermReader_(NULL)
        ,pInputDescriptor_(NULL)
        ,ownPosting_(false)
{
    termInfo_.set(pPosting->docFreq(), 0);
}

TermDocFreqs::~TermDocFreqs(void)
{
    close();
    if (pPostingBuffer_)
    {
        delete[] pPostingBuffer_;
        pPostingBuffer_ = NULL;
    }
    nBufferSize_ = 0;
}

freq_t TermDocFreqs::docFreq()
{
    return termInfo_.docFreq();
}

int64_t TermDocFreqs::getCTF()
{
    return pPosting_->getCTF();
}

docid_t TermDocFreqs::doc()
{
    return pPostingBuffer_[nCurrentPosting_];
}

count_t TermDocFreqs::freq()
{
    return pPostingBuffer_[nFreqStart_ + nCurrentPosting_];
}

bool TermDocFreqs::next()
{
    nCurrentPosting_ ++;
    if (nCurrentPosting_ >= nCurDecodedCount_)
    {
        if (!decode())
            return false;
        return true;
    }
    return true;
}

docid_t TermDocFreqs::skipTo(docid_t docId)
{
    int32_t start,end;
    while (true)
    {
        if((nCurrentPosting_ == -1) || (nCurrentPosting_ >= nCurDecodedCount_) )
        {
            if(!pPostingBuffer_)
                createBuffer();
            nCurrentPosting_ = 0;
            nCurDecodedCount_ = 1;
            pPostingBuffer_[0] = pPosting_->decodeTo(docId);
            return pPostingBuffer_[0];
        }
        start = nCurrentPosting_;
        end = start + (docId - pPostingBuffer_[nCurrentPosting_]);
        if(end == start)
        {
            return docId;
        }
        else if(end < start)
        {
            return pPostingBuffer_[nCurrentPosting_];			
        }				
        if ( end > (nCurDecodedCount_ - 1) )
            end = nCurDecodedCount_ - 1;
        docid_t nRetDocId;
        nCurrentPosting_ = bsearch(pPostingBuffer_,start,end,docId,nRetDocId);///binary search in decoded buffer
        if(nRetDocId >= docId)
            return nRetDocId;
        nCurrentPosting_ = nCurDecodedCount_;///buffer is over
    }
}

int32_t TermDocFreqs::bsearch(docid_t docs[],int32_t start,int32_t end,docid_t key,docid_t& keyFound)
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
    if (pInputDescriptor_)
    {
        delete pInputDescriptor_;
        pInputDescriptor_ = NULL;
        pTermReader_ = NULL;
    }
    if (pPosting_)
    {
        if(ownPosting_)
            delete pPosting_;
        pPosting_ = NULL;
    }
}
bool TermDocFreqs::decode()
{
    count_t df = termInfo_.docFreq();
    if ((count_t)nTotalDecodedCount_ == df)
    {
        return false;
    }

    if (!pPostingBuffer_)
        createBuffer();

    nCurrentPosting_ = 0;
    nCurDecodedCount_ = pPosting_->decodeNext(pPostingBuffer_,nBufferSize_);
    if (nCurDecodedCount_ <= 0)
        return false;
    nTotalDecodedCount_ += nCurDecodedCount_;

    return true;
}
void TermDocFreqs::createBuffer()
{
    size_t bufSize = TermDocFreqs::DEFAULT_BUFFERSIZE;
    if (pPostingBuffer_ == NULL)
    {
        nBufferSize_ = bufSize;
        nFreqStart_ = nBufferSize_/2;
        pPostingBuffer_ = new uint32_t[nBufferSize_];
        memset(pPostingBuffer_, 0, nBufferSize_*sizeof(uint32_t));
    }
}

