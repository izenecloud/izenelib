#include <ir/index_manager/index/TermIterator.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/TermReader.h>

#include <sstream>

using namespace std;

using namespace izenelib::ir::indexmanager;

TermIterator::TermIterator(void)
        :pBuffer(NULL)
        ,nBuffSize(0)
{
}

TermIterator::~TermIterator(void)
{
    pBuffer = NULL;
    nBuffSize = 0;
}
size_t TermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    this->pBuffer = pBuffer;
    nBuffSize = bufSize;
    return bufSize;
}

BasicDiskTermIterator::BasicDiskTermIterator(DiskTermReader* termReader)
        :pTermReader(termReader)
        ,pCurTerm(NULL)
        ,pCurTermInfo(NULL)
        ,pCurTermPosting(NULL)
        ,pInputDescriptor(NULL)
{
}

BasicDiskTermIterator::~BasicDiskTermIterator(void)
{
    if (pCurTerm)
    {
        delete pCurTerm;
        pCurTerm = NULL;
    }
    if (pCurTermPosting)
    {
        delete pCurTermPosting;
        pCurTermPosting = NULL;
    }
    if (pInputDescriptor)
    {
        delete pInputDescriptor;
        pInputDescriptor = NULL;
    }
    pCurTermInfo = NULL;
}

const Term* BasicDiskTermIterator::term()
{
    return pCurTerm;
}
const TermInfo* BasicDiskTermIterator::termInfo()
{
    return pCurTermInfo;
}
Posting* BasicDiskTermIterator::termPosting()
{
    if (!pCurTermPosting)
    {
        IndexInput* pInput;
        pInputDescriptor = new InputDescriptor(true);
        pInput = pTermReader->getTermReaderImpl()->pInputDescriptor->getDPostingInput()->clone();
        pInputDescriptor->setDPostingInput(pInput);
        pInput = pTermReader->getTermReaderImpl()->pInputDescriptor->getPPostingInput()->clone();
        pInputDescriptor->setPPostingInput(pInput);
        pCurTermPosting = new OnDiskPosting(pInputDescriptor,pCurTermInfo->docPointer());
    }
    else
    {
        ((OnDiskPosting*)pCurTermPosting)->reset(pCurTermInfo->docPointer());///reset to a new posting
    }
    return pCurTermPosting;
}

size_t BasicDiskTermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    int64_t nDLen,nPLen;
    pTermReader->getFieldInfo()->getLength(NULL,&nDLen,&nPLen);
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

//////////////////////////////////////////////////////////////////////////
//
DiskTermIterator::DiskTermIterator(DiskTermReader* termReader)
        :BasicDiskTermIterator(termReader)
        ,currTermIter(termReader->getTermReaderImpl()->pTermTable->begin())
        ,termIterEnd(termReader->getTermReaderImpl()->pTermTable->end())
{
}

bool DiskTermIterator::next()
{
    if(currTermIter != termIterEnd)
    {
        if (pCurTerm == NULL)
            pCurTerm = new Term(pTermReader->getFieldInfo()->getName(),currTermIter->first);
        else pCurTerm->setValue(currTermIter->first);

        pCurTermInfo = &(currTermIter->second);
        ++currTermIter;
        return true;
    }
    else return false;
}

//////////////////////////////////////////////////////////////////////////
//
UnOrderedDiskTermIterator::UnOrderedDiskTermIterator(DiskTermReader* termReader)
        :BasicDiskTermIterator(termReader)
        ,currTermIter(termReader->getTermTable()->begin())
        ,termIterEnd(termReader->getTermTable()->end())
{
}

bool UnOrderedDiskTermIterator::next()
{
    if(currTermIter != termIterEnd)
    {
        if (pCurTerm == NULL)
            pCurTerm = new Term(pTermReader->getFieldInfo()->getName(),currTermIter->first);
        else pCurTerm->setValue(currTermIter->first);

        pCurTermInfo = &(currTermIter->second);
        ++currTermIter;
        return true;
    }
    else return false;
}

//////////////////////////////////////////////////////////////////////////
//
InMemoryTermIterator::InMemoryTermIterator(InMemoryTermReader* pTermReader)
        :pTermReader(pTermReader)
        ,pCurTerm(NULL)
        ,pCurTermInfo(NULL)
        ,pCurTermPosting(NULL)
        ,postingIterator(pTermReader->pIndexer->postingMap_.begin())
        ,postingIteratorEnd(pTermReader->pIndexer->postingMap_.end())
{
}

InMemoryTermIterator::~InMemoryTermIterator(void)
{
    if (pCurTerm)
    {
        delete pCurTerm;
        pCurTerm = NULL;
    }
    pCurTermPosting = NULL;
    if (pCurTermInfo)
    {
        delete pCurTermInfo;
        pCurTermInfo = NULL;
    }
}

bool InMemoryTermIterator::next()
{
    postingIterator++;

    if(postingIterator != postingIteratorEnd)
    {
        pCurTermPosting = postingIterator->second;
        while (pCurTermPosting->hasNoChunk())
        {
            postingIterator++;
            if(postingIterator != postingIteratorEnd)
                pCurTermPosting = postingIterator->second;
            else return false;
        }

        //TODO
        if (pCurTerm == NULL)
            pCurTerm = new Term(pTermReader->sField.c_str(),postingIterator->first);
        else pCurTerm->setValue(postingIterator->first);
        pCurTermPosting = postingIterator->second;
        if (pCurTermInfo == NULL)
            pCurTermInfo = new TermInfo(pCurTermPosting->docFreq(),-1);
        else
            pCurTermInfo->set(pCurTermPosting->docFreq(),-1);
        return true;
    }
    else return false;
}

const Term* InMemoryTermIterator::term()
{
    return pCurTerm;
}
const TermInfo* InMemoryTermIterator::termInfo()
{
    return pCurTermInfo;
}
Posting* InMemoryTermIterator::termPosting()
{
    return pCurTermPosting;
}
size_t InMemoryTermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    return 0;///don't need buffer
}

