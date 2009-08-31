#include <ir/index_manager/index/TermReader.h>

#include <boost/thread.hpp>

using namespace izenelib::ir::indexmanager;

TermReader::TermReader(void)
        : pFieldInfo(NULL)
{

}

TermReader::TermReader(FieldInfo* pFieldInfo_)
        : pFieldInfo(pFieldInfo_)
{

}

TermReader::~TermReader(void)
{

}

void TermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{}

DiskTermReader::DiskTermReader()
        : TermReader()
        , pTermReaderImpl(NULL)
        , ownTermReaderImpl(true)
{
}

DiskTermReader::DiskTermReader(TermReaderImpl* pTermReaderImpl_)
        : TermReader()
        , pTermReaderImpl(pTermReaderImpl_)
        , ownTermReaderImpl(false)
{

}

TermReaderImpl::TermReaderImpl(FieldInfo* pFieldInfo_)
        :pFieldInfo(pFieldInfo_)
        ,pTermTable(NULL)
        ,pInputDescriptor(NULL)
{}

TermReaderImpl::~TermReaderImpl()
{
    close();
}

DiskTermReader::~DiskTermReader()
{
    close();
    if ((pTermReaderImpl)&&(ownTermReaderImpl))
        delete pTermReaderImpl;
}

void DiskTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    setFieldInfo(pFieldInfo);

    pTermReaderImpl = new TermReaderImpl(pFieldInfo);
    pTermReaderImpl->open(pDirectory, barrelname, pFieldInfo);
}

void TermReaderImpl::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    close();///TODO

    string bn = barrelname;

    IndexInput* pVocInput = pDirectory->openInput(bn + ".voc");
    pVocInput->seek(pFieldInfo->getIndexOffset());
    fileoffset_t voffset = pVocInput->getFilePointer();
    ///begin read vocabulary descriptor
    nVocLength = pVocInput->readLong();
    nTermCount = (int32_t)pVocInput->readLong(); ///get total term count
    ///end read vocabulary descriptor
    pVocInput->seek(voffset - nVocLength);///seek to begin of vocabulary data
    pTermTable = new TERM_TABLE;//[nTermCount];
    freq_t df = 0;
    fileoffset_t dfiP = 0;
    termid_t tid = 0;
    ///read term table
    for (int32_t i = 0;i < nTermCount;i++)
    {
        tid = pVocInput->readInt();
        //pTermTable[i].tid = tid;
        df = pVocInput->readInt();
        dfiP = pVocInput->readLong();
        //pTermTable[i].ti.set(df,dfiP);
        TermInfo ti;
        ti.set(df,dfiP);
        (*pTermTable)[tid] = ti;
    }
    delete pVocInput;

    pInputDescriptor = new InputDescriptor(true);
    pInputDescriptor->setDPostingInput(pDirectory->openInput(bn + ".dfp"));
    pInputDescriptor->setPPostingInput(pDirectory->openInput(bn + ".pop"));

}

void TermReaderImpl::close()
{
    if (pInputDescriptor)
    {
        delete pInputDescriptor;
        pInputDescriptor = NULL;
    }

    if (pTermTable)
    {
        //delete[] pTermTable;
        delete pTermTable;
        pTermTable = NULL;
        nTermCount = 0;
    }
}

void DiskTermReader::close()
{
    pCurTermInfo = NULL;
}

bool DiskTermReader::seek(Term* term)
{
    pCurTermInfo = termInfo(term);
    if (pCurTermInfo)
        return true;
    return false;
}

TermDocFreqs* DiskTermReader::termDocFreqs()
{
    if (pCurTermInfo == NULL)
        return NULL;

    TermDocFreqs* pTermDocs = new TermDocFreqs(this,pTermReaderImpl->pInputDescriptor->clone(),*pCurTermInfo);
    return pTermDocs;
}

TermPositions* DiskTermReader::termPositions()
{
    if (pCurTermInfo == NULL || pTermReaderImpl->pInputDescriptor->getPPostingInput() == NULL)
        return NULL;
    return new TermPositions(this,pTermReaderImpl->pInputDescriptor->clone(),*pCurTermInfo);
}


freq_t DiskTermReader::docFreq(Term* term)
{
    TermInfo* ti = termInfo(term);
    if (ti)
    {
        return ti->docFreq();
    }
    return 0;
}

TermIterator* DiskTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (!strcasecmp(getFieldInfo()->getName(),field)))
        return NULL;
    return new DiskTermIterator(this);
}

void TermReaderImpl::updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset)
{
    termid_t tid = term->getValue();

    TERM_TABLE::iterator termTableIter = pTermTable->find(tid);
    termTableIter->second.set(docFreq, offset);
/*
    int32_t start = 0,end = nTermCount - 1;
    int32_t mid = (start + end)/2;
    while (start <= end)
    {
        mid = (start + end)/2;
        if (pTermTable[mid].tid == tid)
        {
            pTermTable[mid].ti.set(docFreq, offset);
            return;
        }
        if (pTermTable[mid].tid > tid)
        {
            end = mid - 1;
        }
        else
        {
            start = mid + 1;
        }
    }
*/
}

void DiskTermReader::updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset)
{
    pTermReaderImpl->updateTermInfo(term, docFreq, offset);
}

TermInfo* TermReaderImpl::termInfo(Term* term)
{
    if (strcmp(term->getField(),pFieldInfo->getName()))
        return NULL;

    termid_t tid = term->getValue();

    TERM_TABLE::iterator termTableIter = pTermTable->find(tid);
    if(termTableIter != pTermTable->end())
        return &(termTableIter->second);
    else
        return NULL;
/*	
    int32_t start = 0,end = nTermCount - 1;
    int32_t mid = (start + end)/2;
    while (start <= end)
    {
        mid = (start + end)/2;
        if (pTermTable[mid].tid == tid)
        {
            return &(pTermTable[mid].ti);
        }
        if (pTermTable[mid].tid > tid)
        {
            end = mid - 1;
        }
        else
        {
            start = mid + 1;
        }
    }
    return NULL;
*/	
}

TermInfo* DiskTermReader::termInfo(Term* term)
{
    return pTermReaderImpl->termInfo(term);
}

TermReader* DiskTermReader::clone()
{
    return new DiskTermReader(pTermReaderImpl);
}


//////////////////////////////////////////////////////////////////////////
///InMemoryTermReader
InMemoryTermReader::InMemoryTermReader(void)
        : pIndexer(NULL)
        , pCurTermInfo(NULL)
        , pCurPosting(NULL)
        , pTermInfo(NULL)
{
}
InMemoryTermReader::InMemoryTermReader(const char* field,FieldIndexer* pIndexer_)
        : sField(field)
        , pIndexer(pIndexer_)
        , pCurTermInfo(NULL)
        , pCurPosting(NULL)
        , pTermInfo(NULL)
{
}

InMemoryTermReader::~InMemoryTermReader(void)
{
    close();
}

void InMemoryTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{

}

TermIterator* InMemoryTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (sField != field))
        return NULL;
    return new InMemoryTermIterator(this);
}

bool InMemoryTermReader::seek(Term* term)
{
    pCurTermInfo = termInfo(term);
    if (pCurTermInfo)
        return true;
    return false;
}

TermDocFreqs* InMemoryTermReader::termDocFreqs()
{
    if (pCurTermInfo == NULL)
        return NULL;

    InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting;
    boost::mutex::scoped_lock lock(pIndexer->getLock());
    pInMem->flushLastDoc(false);
    TermDocFreqs* pTermDocs = new TermDocFreqs(this,pCurPosting,*pCurTermInfo);
    return pTermDocs;
}

TermPositions* InMemoryTermReader::termPositions()
{
    if (pCurTermInfo == NULL)
        return NULL;

    InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting;
    boost::mutex::scoped_lock lock(pIndexer->getLock());
    pInMem->flushLastDoc(false);
    return new TermPositions(this,pCurPosting,*pCurTermInfo);
}
freq_t InMemoryTermReader::docFreq(Term* term)
{
    TermInfo* ti = termInfo(term);
    if (ti)
    {
        return ti->docFreq();
    }
    return 0;
}

TermInfo* InMemoryTermReader::termInfo(Term* term)
{
    if (strcasecmp(term->getField(),sField.c_str()))
        return NULL;

    termid_t tid = term->getValue();
    if (tid < 0)
        return NULL;
    pCurPosting = pIndexer->postingMap_[tid];
    if (!pCurPosting || (pCurPosting->hasNoChunk() == true))
        return NULL;
    if (!pTermInfo)
        pTermInfo = new TermInfo;
    pTermInfo->set(pCurPosting->docFreq(),-1);
    return pTermInfo;
}

InMemoryPosting* InMemoryTermReader::inMemoryPosting()
{
    if (!pCurPosting || (pCurPosting->hasNoChunk() == true))
        return NULL;
    return pCurPosting;
}

void InMemoryTermReader::close()
{
    if (pTermInfo)
    {
        delete pCurTermInfo;
        pCurTermInfo = NULL;
    }
    pCurTermInfo = NULL;
}


TermReader* InMemoryTermReader::clone()
{
    return new InMemoryTermReader(sField.c_str(), pIndexer);
}

