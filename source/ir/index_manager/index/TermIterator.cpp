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

DiskTermIterator::DiskTermIterator(DiskTermReader* termReader)
        :pTermReader(termReader)
        ,pCurTerm(NULL)
        ,pCurTermInfo(NULL)
        ,pCurTermPosting(NULL)
        ,pInputDescriptor(NULL)
        ,nCurPos(-1)
{
}

DiskTermIterator::~DiskTermIterator(void)
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

bool DiskTermIterator::next()
{
    if (pTermReader->getTermReaderImpl()->nTermCount > nCurPos+1)
    {
        if (pCurTerm == NULL)
            pCurTerm = new Term(pTermReader->getFieldInfo()->getName(),pTermReader->getTermReaderImpl()->pTermTable[++nCurPos].tid);
        else pCurTerm->setValue(pTermReader->getTermReaderImpl()->pTermTable[++nCurPos].tid);
        pCurTermInfo = &(pTermReader->getTermReaderImpl()->pTermTable[nCurPos].ti);
        return true;
    }
    else return false;
}

bool DiskTermIterator::skipTo(const Term* target)
{
    /*bool ret = termIterator.skipTo((target)->getValue());
    if(ret)
    {
        if(pCurTerm == NULL)
            pCurTerm = new Term(field.c_str(),termIterator.term());
        else pCurTerm->setValue(termIterator.term());
    }*/
    ///TODO:
    return false;
}
const Term* DiskTermIterator::term()
{
    return pCurTerm;
}
const TermInfo* DiskTermIterator::termInfo()
{
    return pCurTermInfo;
}
Posting* DiskTermIterator::termPosting()
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
freq_t DiskTermIterator::docFreq()
{
    return pCurTermInfo->docFreq();
}

size_t DiskTermIterator::setBuffer(char* pBuffer,size_t bufSize)
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
InMemoryTermIterator::InMemoryTermIterator(InMemoryTermReader* pTermReader)
        :pTermReader(pTermReader)
        ,pCurTerm(NULL)
        ,pCurTermInfo(NULL)
        ,pCurTermPosting(NULL)
        ,aIterator(pTermReader->pIndexer->array_.elements())
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
    bool bret = aIterator.next();
    if (bret)
    {
        pCurTermPosting = aIterator.element();
        while (pCurTermPosting->hasNoChunk())
        {
            bret = aIterator.next();
            if (bret)
                pCurTermPosting = aIterator.element();
            else return false;
        }
        //TODO
        if (pCurTerm == NULL)
            pCurTerm = new Term(pTermReader->sField.c_str(),(termid_t)aIterator.position());
        else pCurTerm->setValue(aIterator.position());
        pCurTermPosting = aIterator.element();
        if (pCurTermInfo == NULL)
            pCurTermInfo = new TermInfo(pCurTermPosting->docFreq(),-1);
        else
            pCurTermInfo->set(pCurTermPosting->docFreq(),-1);
        return true;
    }
    else return false;
}

bool InMemoryTermIterator::skipTo(const Term* target)
{
    /*bool ret = termIterator.skipTo(target->getValue());
    if(ret)
    {
    if(pCurTerm == NULL)
    pCurTerm = new Term(field.c_str(),termIterator.term());
    else pCurTerm->setValue(termIterator.term());
    }*/
    ///TODO:
    return false;
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
freq_t InMemoryTermIterator::docFreq()
{
    if (!pCurTermInfo)
        return 0;
    return pCurTermInfo->docFreq();
}
size_t InMemoryTermIterator::setBuffer(char* pBuffer,size_t bufSize)
{
    return 0;///don't need buffer
}

