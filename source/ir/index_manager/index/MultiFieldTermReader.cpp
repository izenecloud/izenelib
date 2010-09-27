#include <ir/index_manager/index/MultiFieldTermReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/FieldInfo.h>


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

        if (pInfo->isIndexed()&&pInfo->isForward())
        {
            switch(pBarrelInfo->compressType)
            {
            case BYTE:
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
        return iter->second;//->clone();
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
    return pCurReader_->docFreq(term);
}

TermInfo* MultiFieldTermReader::termInfo(Term* term)
{
    return pCurReader_->termInfo(term);
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
    MultiFieldTermReader* pReader = new MultiFieldTermReader();

    reader_map::iterator iter = fieldsTermReaders_.begin();
    while (iter != fieldsTermReaders_.end())
    {
        TermReader* pFieldTermReader = iter->second->clone();
        pFieldTermReader->setSkipInterval(skipInterval_);
        pFieldTermReader->setMaxSkipLevel(maxSkipLevel_);
        pReader->fieldsTermReaders_.insert(make_pair(iter->first,pFieldTermReader));
        iter++;
    }
    pReader->setSkipInterval(skipInterval_);
    pReader->setMaxSkipLevel(maxSkipLevel_);

    return pReader;
}

void MultiFieldTermReader::addTermReader(const char* field,TermReader* pTermReader)
{
    string sf = field;
    fieldsTermReaders_.insert(make_pair(sf,pTermReader));
}

