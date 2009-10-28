#include <ir/index_manager/index/TermIterator.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/TermReader.h>

#include <sstream>

using namespace std;

using namespace izenelib::ir::indexmanager;

TermIterator::TermIterator(void)
        :pBuffer_(NULL)
        ,nBuffSize_(0)
{
}

TermIterator::~TermIterator(void)
{
    pBuffer_ = NULL;
    nBuffSize_ = 0;
}
size_t TermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    this->pBuffer_ = pBuffer;
    nBuffSize_ = bufSize;
    return bufSize;
}

DiskTermIterator::DiskTermIterator(DiskTermReader* termReader)
        :pTermReader_(termReader)
        ,pCurTerm_(NULL)
        ,pCurTermInfo_(NULL)
        ,pCurTermPosting_(NULL)
        ,pInputDescriptor_(NULL)
        ,currTermIter_(termReader->getTermReaderImpl()->pTermTable_->begin())
        ,termIterEnd_(termReader->getTermReaderImpl()->pTermTable_->end())
{
}

DiskTermIterator::~DiskTermIterator(void)
{
    if (pCurTerm_)
    {
        delete pCurTerm_;
        pCurTerm_ = NULL;
    }
    if (pCurTermPosting_)
    {
        delete pCurTermPosting_;
        pCurTermPosting_ = NULL;
    }
    if (pInputDescriptor_)
    {
        delete pInputDescriptor_;
        pInputDescriptor_ = NULL;
    }
    pCurTermInfo_ = NULL;
}

const Term* DiskTermIterator::term()
{
    return pCurTerm_;
}

const TermInfo* DiskTermIterator::termInfo()
{
    return pCurTermInfo_;
}

Posting* DiskTermIterator::termPosting()
{
    if (!pCurTermPosting_)
    {
        IndexInput* pInput;
        pInputDescriptor_ = new InputDescriptor(true);
        pInput = pTermReader_->getTermReaderImpl()->pInputDescriptor_->getDPostingInput()->clone();
        pInputDescriptor_->setDPostingInput(pInput);
        pInput = pTermReader_->getTermReaderImpl()->pInputDescriptor_->getPPostingInput()->clone();
        pInputDescriptor_->setPPostingInput(pInput);
        pCurTermPosting_ = new OnDiskPosting(pInputDescriptor_,pCurTermInfo_->docPointer());
    }
    else
    {
        ((OnDiskPosting*)pCurTermPosting_)->reset(pCurTermInfo_->docPointer());///reset to a new posting
    }
    return pCurTermPosting_;
}

size_t DiskTermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    int64_t nDLen,nPLen;
    pTermReader_->getFieldInfo()->getLength(NULL,&nDLen,&nPLen);
    if ( (size_t)(nDLen + nPLen) < bufSize)
    {
        TermIterator::setBuffer(pBuffer,(size_t)(nDLen + nPLen));
        return (size_t)(nDLen + nPLen);
    }
    else
    {
        TermIterator::setBuffer(pBuffer,bufSize);
        return bufSize;
    }
}

bool DiskTermIterator::next()
{
    if(currTermIter_ != termIterEnd_)
    {
        if (pCurTerm_ == NULL)
            pCurTerm_ = new Term(pTermReader_->getFieldInfo()->getName(),currTermIter_->first);
        else pCurTerm_->setValue(currTermIter_->first);

        pCurTermInfo_ = &(currTermIter_->second);
        ++currTermIter_;
        return true;
    }
    else return false;
}

//////////////////////////////////////////////////////////////////////////
//
InMemoryTermIterator::InMemoryTermIterator(InMemoryTermReader* pTermReader)
        :pTermReader_(pTermReader)
        ,pCurTerm_(NULL)
        ,pCurTermInfo_(NULL)
        ,pCurTermPosting_(NULL)
        ,postingIterator_(pTermReader->pIndexer_->postingMap_.begin())
        ,postingIteratorEnd_(pTermReader->pIndexer_->postingMap_.end())
{
}

InMemoryTermIterator::~InMemoryTermIterator(void)
{
    if (pCurTerm_)
    {
        delete pCurTerm_;
        pCurTerm_ = NULL;
    }
    pCurTermPosting_ = NULL;
    if (pCurTermInfo_)
    {
        delete pCurTermInfo_;
        pCurTermInfo_ = NULL;
    }
}

bool InMemoryTermIterator::next()
{
    postingIterator_++;

    if(postingIterator_ != postingIteratorEnd_)
    {
        pCurTermPosting_ = postingIterator_->second;
        while (pCurTermPosting_->hasNoChunk())
        {
            postingIterator_++;
            if(postingIterator_ != postingIteratorEnd_)
                pCurTermPosting_ = postingIterator_->second;
            else return false;
        }

        //TODO
        if (pCurTerm_ == NULL)
            pCurTerm_ = new Term(pTermReader_->field_.c_str(),postingIterator_->first);
        else pCurTerm_->setValue(postingIterator_->first);
        pCurTermPosting_ = postingIterator_->second;
        if (pCurTermInfo_ == NULL)
            pCurTermInfo_ = new TermInfo(pCurTermPosting_->docFreq(),-1);
        else
            pCurTermInfo_->set(pCurTermPosting_->docFreq(),-1);
        return true;
    }
    else return false;
}

const Term* InMemoryTermIterator::term()
{
    return pCurTerm_;
}
const TermInfo* InMemoryTermIterator::termInfo()
{
    return pCurTermInfo_;
}
Posting* InMemoryTermIterator::termPosting()
{
    return pCurTermPosting_;
}

size_t InMemoryTermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    return 0;///don't need buffer
}

