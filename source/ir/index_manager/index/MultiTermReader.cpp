#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/MultiTermDocs.h>
#include <ir/index_manager/index/MultiTermPositions.h>
#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermIterator.h>


using namespace izenelib::ir::indexmanager;

MultiTermReader::MultiTermReader(void)
{
}

MultiTermReader::MultiTermReader(MultiIndexBarrelReader* pReader, collectionid_t id)
        : colID(id)
        , pBarrelReader(pReader)
        , pCurReader(NULL)
{
}
MultiTermReader::~MultiTermReader(void)
{
    pBarrelReader = NULL;
    close();
}

void MultiTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    throw UnsupportedOperationException("Unsupported Operation : MultiTermReader::open.");
}


TermIterator* MultiTermReader::termIterator(const char* field)
{
    ReaderCache* pSeList = NULL;
    map<string,ReaderCache*>::iterator iter = readerCache.find(field);
    if (iter != readerCache.end())
    {
        pSeList = iter->second;
    }
    else
    {
        pSeList = loadReader(field);
    }
    MultiTermIterator* pTermIterator = new MultiTermIterator();
    TermIterator* pIt;
    while (pSeList)
    {
        pIt = pSeList->pTermReader->termIterator(field);
        if (pIt)
            pTermIterator->addIterator(pIt);
        pSeList = pSeList->next;
    }
    return pTermIterator;
}

bool MultiTermReader::seek(Term* term)
{
    bool bSuc = false;
    string field = term->getField();
    ReaderCache* pSeList = NULL;

    pCurReader = NULL;
    map<string,ReaderCache*>::iterator iter = readerCache.find(field);
    if (iter != readerCache.end())
    {
        pCurReader = pSeList = iter->second;
    }
    else
    {
        pCurReader = pSeList = loadReader(field.c_str());
    }
    while (pSeList)
    {
        bSuc = (pSeList->pTermReader->seek(term) || bSuc);
        pSeList = pSeList->next;
    }

    return bSuc;
}

TermDocFreqs* MultiTermReader::termDocFreqs()
{
    if (!pCurReader)
        return NULL;

    MultiTermDocs* pTermDocs = new MultiTermDocs();
    bool bAdd = false;
    TermDocFreqs* pTmpTermDocs = NULL;
    ReaderCache* pList = pCurReader;
    while (pList)
    {
        pTmpTermDocs = pList->pTermReader->termDocFreqs();
        if (pTmpTermDocs)
        {
            pTermDocs->add(pList->barrelInfo,pTmpTermDocs);
            bAdd = true;
        }
        pList = pList->next;
    }
    if (bAdd == false)
    {
        delete pTermDocs;
        return NULL;
    }
    return pTermDocs;
}

TermPositions* MultiTermReader::termPositions()
{
    if (!pCurReader)
        return NULL;
    MultiTermPositions* pTermPositions = new MultiTermPositions();
    bool bAdd = false;
    TermPositions* pTmpTermPositions = NULL;
    ReaderCache* pList = pCurReader;
    while (pList)
    {
        pTmpTermPositions = pList->pTermReader->termPositions();
        if (pTmpTermPositions)
        {
            pTermPositions->add(pList->barrelInfo, pTmpTermPositions);
            bAdd = true;
        }
        pList = pList->next;
    }
    if (bAdd == false)
    {
        delete pTermPositions;
        return NULL;
    }
    return pTermPositions;
}

freq_t MultiTermReader::docFreq(Term* term)
{
    freq_t df = 0;
    string field = term->getField();
    ReaderCache* pSeList = NULL;

    pCurReader = NULL;
    map<string,ReaderCache*>::iterator iter = readerCache.find(field);
    if (iter != readerCache.end())
    {
        pCurReader = pSeList = iter->second;
    }
    else
    {
        pCurReader = pSeList = loadReader(field.c_str());
    }

    while (pSeList)
    {
        df += pSeList->pTermReader->docFreq(term);
        pSeList = pSeList->next;
    }

    return df;
}

void MultiTermReader::close()
{
    map<string,ReaderCache*>::iterator citer = readerCache.begin();
    while (citer != readerCache.end())
    {
        delete citer->second;
        citer++;
    }
    readerCache.clear();
    pCurReader = NULL;
}


TermReader* MultiTermReader::clone()
{
    return new MultiTermReader(pBarrelReader, colID);
}

ReaderCache* MultiTermReader::loadReader(const char* field)
{
    ReaderCache* pTailList = NULL;
    ReaderCache* pPreList = NULL;
    ReaderCache* pHeadList = NULL;
    TermReader* pSe = NULL;
    pBarrelReader->startIterator();
    BarrelReaderEntry* pEntry = NULL;
    while (pBarrelReader->hasNext())
    {
        pEntry = pBarrelReader->nextEntry();
        pSe =  (TermReader*)pEntry->pBarrel->termReader(colID, field)->clone();
        if (pSe)
        {
            pTailList = new ReaderCache(pEntry->pBarrelInfo,pSe);
            if (pHeadList == NULL)
            {
                pHeadList = pTailList;
                pPreList = pTailList;
            }
            else
            {
                pPreList->next = pTailList;
                pPreList = pTailList;
            }
        }
    }
    if (pHeadList)
    {
        readerCache.insert(pair<string,ReaderCache*>(field,pHeadList));
    }
    else SF1V5_LOG(level::warn) << "No such field:" << field << SF1V5_ENDL;
    return pHeadList;
}

