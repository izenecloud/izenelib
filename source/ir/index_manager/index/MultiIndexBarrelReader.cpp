#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/Indexer.h>

#include <boost/thread.hpp>

using namespace izenelib::ir::indexmanager;

MultiIndexBarrelReader::MultiIndexBarrelReader(
    IndexReader* pIndexReader,
    BarrelsInfo* pBarrelsInfo)
    :IndexBarrelReader(pIndexReader)
    ,pBarrelsInfo_(pBarrelsInfo)
    ,hasMemBarrel_(false)
{
    boost::mutex::scoped_lock lock(pBarrelsInfo_->getMutex());

    pBarrelsInfo_->startIterator();
    while (pBarrelsInfo_->hasNext())
    {
        BarrelInfo* pBarrelInfo = pBarrelsInfo_->next();
        if(!pBarrelInfo->isSearchable())
            continue;
        if (pBarrelInfo->getDocCount() > 0)
            readers_.push_back(new BarrelReaderEntry(pIndexReader_,pBarrelInfo));
    }
    vector<BarrelReaderEntry*>::iterator iter = readers_.begin();
    for(; iter != readers_.end(); ++iter)
        hasMemBarrel_ |= (*iter)->pBarrelReader_->hasMemBarrel();
}

MultiIndexBarrelReader::~MultiIndexBarrelReader()
{
    pBarrelsInfo_ = NULL;
    close();
    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin();
            iter != termReaderMap_.end(); ++iter)
        delete iter->second;
    termReaderMap_.clear();
}

void MultiIndexBarrelReader::open(const char* name)
{
}

TermReader* MultiIndexBarrelReader::termReader(collectionid_t colID)
{
    map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.find(colID);
    if(iter != termReaderMap_.end())
        return iter->second;

    TermReader* pTermReader = new MultiTermReader(this, colID);
    termReaderMap_.insert(pair<collectionid_t, TermReader*>(colID, pTermReader));
    return pTermReader;
}

void MultiIndexBarrelReader::close()
{
    vector<BarrelReaderEntry*>::iterator iter = readers_.begin();
    for(; iter != readers_.end(); ++iter)
        delete (*iter);
    readers_.clear();
}

size_t MultiIndexBarrelReader::getDistinctNumTerms(
    collectionid_t colID, 
    const std::string& property)
{
    size_t num = 0;
    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
        num += (*iter)->pBarrelReader_->getDistinctNumTerms(colID,property);
    return num;
}

