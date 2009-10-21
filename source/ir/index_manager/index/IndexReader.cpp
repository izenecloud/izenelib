#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/MultiIndexBarrelReader.h>


using namespace izenelib::ir::indexmanager;

IndexReader::IndexReader(Indexer* pIndex, DiskIndexOpenMode openMode)
        :openMode_(openMode)
        ,pIndexer_(pIndex)
        ,pBarrelsInfo_(NULL)
        ,pBarrelReader_(NULL)
        ,pForwardIndexReader_(NULL)
        ,dirty_(false)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();
}

IndexReader::~IndexReader(void)
{
    if (pBarrelReader_)
    {
        delete pBarrelReader_;
        pBarrelReader_ = NULL;
    }
    if(pForwardIndexReader_)
        delete pForwardIndexReader_;
}

void IndexReader::createBarrelReader()
{
    if (pBarrelReader_)
        delete pBarrelReader_;
    int32_t bc = pBarrelsInfo_->getBarrelCount();
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    if ( (bc > 0) && pLastBarrel)
    {
        if (pLastBarrel->getDocCount() <= 0)///skip empty barrel
            bc--;
        pLastBarrel = (*pBarrelsInfo_)[bc - 1];
    }

    if (bc == 1)
    {
        if (pLastBarrel && pLastBarrel->getWriter())
            pBarrelReader_ = pLastBarrel->getWriter()->inMemoryReader();
        else
            pBarrelReader_ = new SingleIndexBarrelReader(pIndexer_,pLastBarrel,openMode_);
    }
    else if (bc > 1)
    {
        pBarrelReader_ = new MultiIndexBarrelReader(pIndexer_,pBarrelsInfo_,openMode_);
    }
    else
        SF1V5_THROW(ERROR_INDEX_COLLAPSE,"the index barrel number is 0.");
    pIndexer_->setDirty(false);///clear dirty_ flag

}

TermReader* IndexReader::doGetTermReader_(collectionid_t colID)
{
    if (dirty_)
    {
        //collection has been removed, need to rebuild the barrel reader
        if (pBarrelReader_ == NULL)
        {
            createBarrelReader();
        }
        else
        {
            delete pBarrelReader_;
            pBarrelReader_ = NULL;
        }
        dirty_ = false;
    }

    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }

    return pBarrelReader_->termReader(colID);
}

TermReader* IndexReader::getTermReader(collectionid_t colID)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    TermReader* pTermReader = doGetTermReader_(colID);
    if (pTermReader)
        return pTermReader->clone();
    else
        return NULL;
}

ForwardIndexReader* IndexReader::getForwardIndexReader()
{
    if(!pForwardIndexReader_)
        pForwardIndexReader_ = new ForwardIndexReader(pIndexer_->getDirectory());

    return pForwardIndexReader_->clone();
}

// count_t IndexReader::maxDoc()
// {
//     return pBarrelsInfo_->getDocCount();
// }
count_t IndexReader::numDocs()
{
    return pBarrelsInfo_->getDocCount();
}

void IndexReader::deleteDocumentPhysically(IndexerDocument* pDoc)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }
    pBarrelReader_->deleteDocumentPhysically(pDoc);
    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    pDoc->getPropertyList(propertyValueList);
    DocId uniqueID;
    pDoc->getDocId(uniqueID);
    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if(!iter->first.isIndex())
            continue;    
        if (!iter->first.isForward())
        {
            pIndexer_->getBTreeIndexer()->remove(uniqueID.colId, iter->first.getPropertyId(), boost::get<PropertyType>(iter->second), uniqueID.docId);
        }
    }
    pIndexer_->setDirty(true);//flush barrelsinfo when Indexer quit
}

freq_t IndexReader::docFreq(collectionid_t colID, Term* term)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    TermReader* pTermReader = doGetTermReader_(colID);
    if (pTermReader)
    {
        return pTermReader->docFreq(term);
    }

    return 0;
}

TermInfo* IndexReader::termInfo(collectionid_t colID,Term* term)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    TermReader* pTermReader = doGetTermReader_(colID);
    if (pTermReader)
    {
        return pTermReader->termInfo(term);
    }

    return 0;
}

size_t IndexReader::getDistinctNumTerms(collectionid_t colID, fieldid_t fid)
{
    if (dirty_)
    {
        //collection has been removed, need to rebuild the barrel reader
        if (pBarrelReader_ == NULL)
        {
            createBarrelReader();
        }
        else
        {
            delete pBarrelReader_;
            pBarrelReader_ = NULL;
        }
        dirty_ = false;
    }

    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }
    return pBarrelReader_->getDistinctNumTerms(colID, fid);
}

