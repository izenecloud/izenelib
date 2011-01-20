#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/MultiTermDocs.h>
#include <ir/index_manager/index/MultiTermPositions.h>
#include <ir/index_manager/index/MultiTermIterator.h>

#include <memory> // auto_ptr

using namespace izenelib::ir::indexmanager;

MultiTermReader::MultiTermReader(MultiIndexBarrelReader* pReader, collectionid_t id)
        : colID_(id)
        , pBarrelReader_(pReader)
        , pCurReader_(NULL)
{
}

MultiTermReader::~MultiTermReader(void)
{
    pBarrelReader_ = NULL;
    close();
}

void MultiTermReader::open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{
}

void MultiTermReader::reopen()
{
    ReaderCache* pList = pCurReader_;
    while(pList)
    {
        pList->pTermReader_->reopen();
        pList = pList->next_;
    }
}

TermIterator* MultiTermReader::termIterator(const char* field)
{
    ReaderCache* pReaderCache = NULL;
    map<string,ReaderCache*>::iterator iter = readerCache_.find(field);
    if (iter != readerCache_.end())
    {
        pReaderCache = iter->second;
    }
    else
    {
        pReaderCache = loadReader(field);
    }
    MultiTermIterator* pTermIterator = new MultiTermIterator();
    TermIterator* pIt;
    while (pReaderCache)
    {
        pIt = pReaderCache->pTermReader_->termIterator(field);
        if (pIt)
            pTermIterator->addIterator(pIt);
        pReaderCache = pReaderCache->next_;
    }
    return pTermIterator;
}

bool MultiTermReader::seek(Term* term)
{
    bool bSuc = false;
    string field = term->getField();
    ReaderCache* pReaderCache = NULL;

    pCurReader_ = NULL;
    map<string,ReaderCache*>::iterator iter = readerCache_.find(field);
    if (iter != readerCache_.end())
    {
        pCurReader_ = pReaderCache = iter->second;
    }
    else
    {
        pCurReader_ = pReaderCache = loadReader(field.c_str());
    }
    while (pReaderCache)
    {
        bSuc = (pReaderCache->pTermReader_->seek(term) || bSuc);
        pReaderCache = pReaderCache->next_;
    }

    return bSuc;
}

TermDocFreqs* MultiTermReader::termDocFreqs()
{
    if (!pCurReader_)
        return NULL;

    // use auto_ptr in case of memory leak when exception is thrown in below TermReader::termDocFreqs()
    std::auto_ptr<MultiTermDocs> termDocsPtr(new MultiTermDocs());
    bool bAdd = false;
    TermDocFreqs* pTmpTermDocs = NULL;
    ReaderCache* pList = pCurReader_;
    while (pList)
    {
        pTmpTermDocs = pList->pTermReader_->termDocFreqs();
        if (pTmpTermDocs)
        {
            termDocsPtr->add(pList->pBarrelInfo_,pTmpTermDocs);
            bAdd = true;
        }
        pList = pList->next_;
    }

    if (bAdd == false)
        return NULL;

    return termDocsPtr.release();
}

TermPositions* MultiTermReader::termPositions()
{
    if (!pCurReader_)
        return NULL;

    // use auto_ptr in case of memory leak when exception is thrown in below TermReader::termPositions()
    std::auto_ptr<MultiTermPositions> termPositionsPtr(new MultiTermPositions());
    bool bAdd = false;
    TermPositions* pTmpTermPositions = NULL;
    ReaderCache* pList = pCurReader_;
    while (pList)
    {
        pTmpTermPositions = pList->pTermReader_->termPositions();
        if (pTmpTermPositions)
        {
            termPositionsPtr->add(pList->pBarrelInfo_, pTmpTermPositions);
            bAdd = true;
        }
        pList = pList->next_;
    }

    if (bAdd == false)
        return NULL;

    return termPositionsPtr.release();
}

freq_t MultiTermReader::docFreq(Term* term)
{
    freq_t df = 0;
    string field = term->getField();
    ReaderCache* pReaderCache = NULL;

    pCurReader_ = NULL;
    map<string,ReaderCache*>::iterator iter = readerCache_.find(field);
    if (iter != readerCache_.end())
    {
        pCurReader_ = pReaderCache = iter->second;
    }
    else
    {
        pCurReader_ = pReaderCache = loadReader(field.c_str());
    }

    while (pReaderCache)
    {
        df += pReaderCache->pTermReader_->docFreq(term);
        pReaderCache = pReaderCache->next_;
    }

    return df;
}

void MultiTermReader::close()
{
    for(map<string,ReaderCache*>::iterator iter = readerCache_.begin(); 
            iter != readerCache_.end(); ++iter)
        delete iter->second;
    pCurReader_ = NULL;
}

TermReader* MultiTermReader::clone()
{
    return new MultiTermReader(pBarrelReader_, colID_);
}

ReaderCache* MultiTermReader::loadReader(const char* field)
{
    DVLOG(5) << "=> MultiTermReader::loadReader(), field: " << field
             << ", pBarrelReader_: " << pBarrelReader_;

    ReaderCache* pTail = NULL;
    // use auto_ptr in case of memory leak when exception is thrown in below for loop
    std::auto_ptr<ReaderCache> headPtr;

    for(vector<BarrelReaderEntry*>::iterator iter = pBarrelReader_->readers_.begin(); 
            iter != pBarrelReader_->readers_.end(); ++iter)	
    {
        if(TermReader* pTermReader = (*iter)->pBarrelReader_->termReader(colID_, field))
        {
            if(TermReader* pSe = pTermReader->clone())
            {
                ReaderCache* pReaderCache = new ReaderCache((*iter)->pBarrelInfo_,pSe);
                if (headPtr.get() == NULL)
                {
                    headPtr.reset(pReaderCache);
                    pTail = pReaderCache;
                }
                else
                {
                    pTail->next_ = pReaderCache;
                    pTail = pReaderCache;
                }
            }
        }
    }

    ReaderCache* pHead = headPtr.release();
    if (pHead)
        readerCache_.insert(make_pair(field, pHead));

    DVLOG(5) << "<= MultiTermReader::loadReader()";
    return pHead;
}

