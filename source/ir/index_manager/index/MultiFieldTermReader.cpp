#include <ir/index_manager/index/MultiFieldTermReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/FieldInfo.h>

#include <memory> // auto_ptr

using namespace izenelib::ir::indexmanager;

MultiFieldTermReader::MultiFieldTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldsInfo* pFieldsInfo)
        : TermReader()
        , pCurReader_(NULL)
{
    FieldInfo* pInfo = NULL;
    TermReader* pTermReader = NULL;
    pFieldsInfo->startIterator();
    while (pFieldsInfo->hasNext())
    {
        pInfo = pFieldsInfo->next();

        if (pInfo->isIndexed()&&pInfo->isAnalyzed())
        {
            switch(pBarrelInfo->compressType)
            {
            case BYTEALIGN:
                pTermReader = new RTDiskTermReader(pDirectory,pBarrelInfo,pInfo);
                break;
            case BLOCK:
                pTermReader = new BlockTermReader(pDirectory,pBarrelInfo,pInfo);
                break;
            case CHUNK:
                pTermReader = new ChunkTermReader(pDirectory,pBarrelInfo,pInfo);
                break;
            default:
                assert(false);
            }
            fieldsTermReaders_.insert(pair<string,TermReader*>(pInfo->getName(),pTermReader));
        }
    }
}

MultiFieldTermReader::MultiFieldTermReader()
        : TermReader()
        , pCurReader_(NULL)
{
}

MultiFieldTermReader::~MultiFieldTermReader()
{
    close();
}

void MultiFieldTermReader::setDocFilter(BitVector* pFilter) 
{
    for(map<string,TermReader*>::iterator iter = fieldsTermReaders_.begin();
        iter != fieldsTermReaders_.end(); ++iter)
        iter->second->setDocFilter(pFilter);
}

void MultiFieldTermReader::open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{}

void MultiFieldTermReader::reopen()
{
    for(reader_map::iterator iter = fieldsTermReaders_.begin(); 
            iter != fieldsTermReaders_.end(); ++iter)
        iter->second->reopen();
}

TermReader* MultiFieldTermReader::termReader(const char* field)
{
    reader_map::iterator iter = fieldsTermReaders_.find(field);
    if (iter != fieldsTermReaders_.end())
    {
        return iter->second;
    }
    return NULL;
}


TermIterator* MultiFieldTermReader::termIterator(const char* field)
{
    map<string,TermReader*>::iterator iter = fieldsTermReaders_.find(field);
    if (iter != fieldsTermReaders_.end())
        return iter->second->termIterator(field);
    return NULL;
}

bool MultiFieldTermReader::seek( Term* term)
{
    string field = term->getField();
    map<string,TermReader*>::iterator iter = fieldsTermReaders_.find(field);
    if (iter != fieldsTermReaders_.end())
    {
        pCurReader_ = iter->second;
        return pCurReader_->seek(term);
    }
    return false;
}

TermDocFreqs* MultiFieldTermReader::termDocFreqs()
{
    return pCurReader_->termDocFreqs();
}

TermPositions* MultiFieldTermReader::termPositions()
{
    return pCurReader_->termPositions();
}


freq_t MultiFieldTermReader::docFreq(Term* term)
{
    if(TermReader* pTermReader = termReader(term->getField()))
        return pTermReader->docFreq(term);
    return NULL;
}

TermInfo* MultiFieldTermReader::termInfo(Term* term)
{
    if(TermReader* pTermReader = termReader(term->getField()))
        return pTermReader->termInfo(term);
    return NULL;
}

void MultiFieldTermReader::close()
{
    pCurReader_ = NULL;;

    reader_map::iterator iter = fieldsTermReaders_.begin();
    while (iter != fieldsTermReaders_.end())
    {
        delete iter->second;
        iter++;
    }
    fieldsTermReaders_.clear();
}


TermReader* MultiFieldTermReader::clone()
{
    // use auto_ptr in case of memory leak when exception is thrown in TermReader::clone()
    std::auto_ptr<MultiFieldTermReader> readerPtr(new MultiFieldTermReader());

    for(reader_map::iterator iter = fieldsTermReaders_.begin();
            iter != fieldsTermReaders_.end(); ++iter)
    {
        TermReader* pFieldTermReader = iter->second->clone();
        pFieldTermReader->setSkipInterval(skipInterval_);
        pFieldTermReader->setMaxSkipLevel(maxSkipLevel_);
        readerPtr->fieldsTermReaders_.insert(make_pair(iter->first,pFieldTermReader));
    }
    readerPtr->setSkipInterval(skipInterval_);
    readerPtr->setMaxSkipLevel(maxSkipLevel_);

    return readerPtr.release();
}

void MultiFieldTermReader::addTermReader(const char* field,TermReader* pTermReader)
{
    string sf = field;
    fieldsTermReaders_.insert(make_pair(sf,pTermReader));
}

