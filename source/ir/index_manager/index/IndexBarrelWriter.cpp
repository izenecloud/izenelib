#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <util/izene_log.h>
#include <util/ThreadModel.h>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cassert>
#include <fstream>
using namespace izenelib::ir::indexmanager;

IndexBarrelWriter::IndexBarrelWriter(Indexer* pIndex)
    :pBarrelInfo_(NULL)
    ,pIndexer_(pIndex)
    ,pCollectionsInfo_(NULL)
    ,pDirectory_(NULL)
    ,pDocFilter_(0)
    ,numFieldIndexers_(0)
    ,isLastIndexModeRealTime_(false)
{
    pCollectionsInfo_ = new CollectionsInfo();
    pDirectory_ = pIndexer_->getDirectory();
}

IndexBarrelWriter::~IndexBarrelWriter()
{
    if (pCollectionsInfo_)
        delete pCollectionsInfo_;

    for (CollectionIndexerMap::iterator iter = collectionIndexerMap_.begin();
        iter != collectionIndexerMap_.end(); ++iter)
        delete iter->second;
}

void IndexBarrelWriter::createMemCache()
{
    if (!pMemCache_)
        pMemCache_.reset(new MemCache((size_t)pIndexer_->getIndexManagerConfig()->indexStrategy_.memory_));
}

void IndexBarrelWriter::destroyMemCache()
{
    if (pMemCache_)
    {
        pMemCache_.reset();
    }
}

void IndexBarrelWriter::close()
{
    flush(); 
    reset();
}

void IndexBarrelWriter::checkbinlog()
{
    CollectionIndexer* pCollectionIndexer = collectionIndexerMap_[1];
    pCollectionIndexer->checkbinlog();
}

void IndexBarrelWriter::deletebinlog()
{
    CollectionIndexer* pCollectionIndexer = collectionIndexerMap_[1];
    pCollectionIndexer->deletebinlog();
}

void IndexBarrelWriter::addDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);
    CollectionIndexer* pCollectionIndexer = collectionIndexerMap_[uniqueID.colId];
    if (NULL == pCollectionIndexer)
        SF1V5_THROW(ERROR_OUTOFRANGE,"IndexBarrelWriter::addDocument(): collection id does not belong to the range");
    pCollectionIndexer->addDocument(doc);
}

void IndexBarrelWriter::flushDocLen()
{
    CollectionIndexerMap::iterator p = collectionIndexerMap_.begin();
    for (; p != collectionIndexerMap_.end( ); ++p)
        (*p).second->flushDocLen();
}

void IndexBarrelWriter::reset()
{
    CollectionIndexerMap::iterator p = collectionIndexerMap_.begin();
    for (; p != collectionIndexerMap_.end( ); ++p)
        (*p).second->reset();

    pCollectionsInfo_->reset();
    if(pMemCache_)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        int ref_count_for_mem_cache = (1+numFieldIndexers_);
        LOG(INFO)<<"Use count of MemCache ===========>"<<pMemCache_.use_count()<<" ref_count_for_mem_cache "<<ref_count_for_mem_cache<<std::endl;
        //After reset() for each FieldIndexer has been called, the overall ref count of memcache
        //should be 1+numFieldIndexers_; If the ref count is greater than that value, it means
        //the in-memory index is reading by some queries, we should wait until they finished
        //while(pMemCache_.use_count() > ref_count_for_mem_cache )
        if(pMemCache_.use_count() > ref_count_for_mem_cache )
        {
            LOG(WARNING)<<"Wait ===========>"<<pMemCache_.use_count()<<" ref_count_for_mem_cache "<<ref_count_for_mem_cache<<std::endl;
            //TODO Currently, only timed_wait for debug usage
            boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(5000);//5s
            cond_.timed_wait(lock,timeout);
        }
        pMemCache_->flushMem();
    }

    pDocFilter_ = NULL;
}

void IndexBarrelWriter::flush()
{
    if(! pBarrelInfo_)
        return;
    pBarrelInfo_->write(pDirectory_);
}


void IndexBarrelWriter::setCollectionsMeta(
    const std::map<std::string, IndexerCollectionMeta>& collectionsMeta)
{
    collectionid_t colID;
    CollectionInfo* pCollectionInfo = NULL;
    CollectionIndexer* pCollectionIndexer = NULL;

    numFieldIndexers_ = 0;

    std::map<std::string, IndexerCollectionMeta>::const_iterator iter
        = collectionsMeta.begin();
    for (; iter != collectionsMeta.end(); iter++)
    {
        colID = (iter->second).getColId ();
        pCollectionIndexer = new CollectionIndexer(colID, pIndexer_);
        pCollectionIndexer->setSchema((iter->second));
        pCollectionIndexer->setFieldIndexers();
        collectionIndexerMap_.insert(make_pair(colID,pCollectionIndexer));
        pCollectionInfo = new CollectionInfo(colID, pCollectionIndexer->getFieldsInfo());
        pCollectionsInfo_->addCollection(pCollectionInfo);
        numFieldIndexers_ += pCollectionIndexer->getNumFieldIndexers();
    }
}

void IndexBarrelWriter::setIndexMode(bool realtime)
{
    if(!realtime)
    {
        destroyMemCache();
        CollectionIndexerMap::iterator cit = collectionIndexerMap_.begin();
        for(;cit != collectionIndexerMap_.end(); ++cit)
        {
            cit->second->setIndexMode(pMemCache_,realtime);
        }
        isLastIndexModeRealTime_ = false;
        assert(pMemCache_.use_count() == 0);
    }
    else
    {
        if(!isLastIndexModeRealTime_)
        {
            createMemCache();
            CollectionIndexerMap::iterator cit = collectionIndexerMap_.begin();
            for(;cit != collectionIndexerMap_.end(); ++cit)
            {
                cit->second->setIndexMode(pMemCache_,realtime);
            }
        }
        isLastIndexModeRealTime_ = true;
    }
}
