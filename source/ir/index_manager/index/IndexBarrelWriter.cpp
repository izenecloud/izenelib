#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <util/izene_log.h>
#include <util/ThreadModel.h>

#include <boost/thread.hpp>

#include <cassert>

using namespace izenelib::ir::indexmanager;

IndexBarrelWriter::IndexBarrelWriter(Indexer* pIndex,MemCache* pCache)
        :pBarrelInfo_(NULL)
        ,pIndexer_(pIndex)
        ,pMemCache_(pCache)
        ,pCollectionsInfo_(NULL)
        ,pDirectory_(NULL)
        ,pDocFilter_(0)
        ,dirty_(false)

{
    pCollectionsInfo_ = new CollectionsInfo();
    pDirectory_ = pIndexer_->getDirectory();
}

IndexBarrelWriter::~IndexBarrelWriter(void)
{
    pMemCache_ = NULL;

    if (pCollectionsInfo_)
        delete pCollectionsInfo_;

    for (CollectionIndexerMap::iterator iter = collectionIndexerMap_.begin(); 
        iter != collectionIndexerMap_.end(); ++iter)
        delete iter->second;
}

void IndexBarrelWriter::open(BarrelInfo* pInfo)
{
    pBarrelInfo_ = pInfo;
    pCollectionsInfo_->reset();
}
void IndexBarrelWriter::close()
{
    if (cacheEmpty() == false)
    {
        writeCache();
        resetCache();
    }
    pDocFilter_ = NULL;
}

void IndexBarrelWriter::addDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);
    CollectionIndexer* pCollectionIndexer = collectionIndexerMap_[uniqueID.colId];
    if (NULL == pCollectionIndexer)
        SF1V5_THROW(ERROR_OUTOFRANGE,"IndexBarrelWriter::addDocument(): collection id does not belong to the range");
    pCollectionIndexer->addDocument(doc);
    if(dirty_) dirty_ = false;
}

void IndexBarrelWriter::resetCache(bool bResetPosting)
{
    if (bResetPosting)
    {
        for (CollectionIndexerMap::iterator p = collectionIndexerMap_.begin( ); p != collectionIndexerMap_.end( ); ++p)
            (*p).second->reset();
    }

    pMemCache_->flushMem();
}

void IndexBarrelWriter::writeCache()
{
    if(! pBarrelInfo_)
        return;

    dirty_ = true;
    pBarrelInfo_->write(pDirectory_);
}


void IndexBarrelWriter::setCollectionsMeta(const std::map<std::string, IndexerCollectionMeta>& collectionsMeta)
{
    std::map<std::string, IndexerCollectionMeta>::const_iterator iter;
    collectionid_t colID;
    CollectionInfo* pCollectionInfo = NULL;
    CollectionIndexer* pCollectionIndexer = NULL;

    for (iter = collectionsMeta.begin(); iter != collectionsMeta.end(); iter++)
    {
        colID = (iter->second).getColId ();
        pCollectionIndexer = new CollectionIndexer(colID, pMemCache_, pIndexer_);
        pCollectionIndexer->setSchema((iter->second));
        pCollectionIndexer->setFieldIndexers();
        collectionIndexerMap_.insert(make_pair(colID,pCollectionIndexer));
        pCollectionInfo = new CollectionInfo(colID, pCollectionIndexer->getFieldsInfo());
        pCollectionsInfo_->addCollection(pCollectionInfo);
    }
}

