#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/Indexer.h>


using namespace izenelib::ir::indexmanager;

MultiIndexBarrelReader::MultiIndexBarrelReader(IndexReader* pIndexReader,BarrelsInfo* pBarrelsInfo)
        :IndexBarrelReader(pIndexReader)
        ,pBarrelsInfo_(pBarrelsInfo)
{
    BarrelInfo* pBarrelInfo;

    pBarrelsInfo_->startIterator();
    while (pBarrelsInfo_->hasNext())
    {
        pBarrelInfo = pBarrelsInfo_->next();
        if(!pBarrelInfo->isSearchable())
            continue;
        if (pBarrelInfo->getDocCount() > 0)
            readers_.push_back(new BarrelReaderEntry(pIndexReader_,pBarrelInfo));
    }
}

MultiIndexBarrelReader::~MultiIndexBarrelReader(void)
{
    pBarrelsInfo_ = NULL;
    close();
}

void MultiIndexBarrelReader::open(const char* name)
{
}

void MultiIndexBarrelReader::reopen()
{
    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
        (*iter)->pBarrelReader_->reopen();
}

TermReader* MultiIndexBarrelReader::termReader(collectionid_t colID)
{
    if (termReaderMap_.find(colID) == termReaderMap_.end())
    {
        boost::shared_ptr<MultiTermReader > pTermReader(new MultiTermReader(this, colID));
        termReaderMap_[colID] = pTermReader;
    }
    return termReaderMap_[colID].get();
}

void MultiIndexBarrelReader::close()
{
    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
        delete (*iter);
    readers_.clear();
}

size_t MultiIndexBarrelReader::getDistinctNumTerms(collectionid_t colID, const std::string& property)
{
    size_t num = 0;
    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
        num += (*iter)->pBarrelReader_->getDistinctNumTerms(colID,property);
    return num;
}
