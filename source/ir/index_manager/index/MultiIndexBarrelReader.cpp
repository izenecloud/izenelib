#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/Indexer.h>


using namespace izenelib::ir::indexmanager;

MultiIndexBarrelReader::MultiIndexBarrelReader(Indexer* pIndexer,BarrelsInfo* pBarrelsInfo, DiskIndexOpenMode mode)
        :IndexBarrelReader(pIndexer)
        ,pBarrelsInfo_(pBarrelsInfo)
{
    pBarrelsInfo_->startIterator();
    BarrelInfo* pBarrelInfo;
    while (pBarrelsInfo_->hasNext())
    {
        pBarrelInfo = pBarrelsInfo_->next();
        if (pBarrelInfo->getDocCount() > 0)
            addReader(pBarrelInfo, mode);
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

TermReader* MultiIndexBarrelReader::termReader(collectionid_t colID)
{
    if (termReaderMap_.find(colID) == termReaderMap_.end())
    {
        try
        {
            boost::shared_ptr<MultiTermReader > pTermReader(new MultiTermReader(this, colID));
            termReaderMap_[colID] = pTermReader;
        }
        catch (std::bad_alloc& ba)
        {
            string serror = ba.what();
            SF1V5_THROW(ERROR_OUTOFMEM,"MultiIndexBarrelReader::termReader():" + serror);
        }
    }
    return termReaderMap_[colID].get();
}

void MultiIndexBarrelReader::deleteDocumentPhysically(IndexerDocument* pDoc)
{
    DocId uniqueID;
    pDoc->getDocId(uniqueID);

    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
    {
        BarrelInfo* pBarrelInfo = (*iter)->pBarrelInfo_;
        if ((pBarrelInfo->baseDocIDMap.find(uniqueID.colId) != pBarrelInfo->baseDocIDMap.end())&&
                (pBarrelInfo->baseDocIDMap[uniqueID.colId] <= uniqueID.docId))
        {
            (*iter)->pBarrelReader_->deleteDocumentPhysically(pDoc);
            pBarrelInfo->deleteDocument();
            break;
        }
    }
}

void MultiIndexBarrelReader::close()
{
    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
        delete (*iter);
    readers_.clear();
}

void MultiIndexBarrelReader::addReader(BarrelInfo* pBarrelInfo, DiskIndexOpenMode mode)
{
    readers_.push_back(new BarrelReaderEntry(pIndexer,pBarrelInfo,mode));
}

size_t MultiIndexBarrelReader::getDistinctNumTerms(collectionid_t colID, fieldid_t fid)
{
    size_t num = 0;
    for(vector<BarrelReaderEntry*>::iterator iter = readers_.begin(); iter != readers_.end(); ++iter)
        num += (*iter)->pBarrelReader_->getDistinctNumTerms(colID,fid);
    return num;
}
