/**
* @file        IndexBarrelWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index the documents into barrels
*/
#ifndef INDEXBARRELWRITER_H
#define INDEXBARRELWRITER_H

#include <ir/index_manager/index/CollectionIndexer.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexerCollectionMeta.h>
#include <ir/index_manager/index/CollectionInfo.h>
#include <ir/index_manager/index/InMemoryIndexBarrelReader.h>

#include <ir/index_manager/store/Directory.h>

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/utility/Bitset.h>
#include <util/izene_log.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef map<collectionid_t, CollectionIndexer*> CollectionIndexerMap;
class Indexer;
class BarrelInfo;

class IndexBarrelWriter
{
public:
    IndexBarrelWriter(Indexer* pIndexer);

    ~IndexBarrelWriter();
public:
    /**
     * checkbinlog
     */
    void checkbinlog();
    /**
     * add a analyzed IndexerDocument
     * @param doc analyzed document
     */
    void addDocument(IndexerDocument& doc);

    /**
     * determine if the memory cache for indexing is full
     * @return true if cache is full otherwise false.
     */
    bool cacheFull()
    {
        return pMemCache_&&pMemCache_->isGrowed();
    }
    /**
     * set a index barrel
     * @param pInfo index barrel info
    */
    void setBarrelInfo(BarrelInfo* pInfo)
    {
        pBarrelInfo_ = pInfo;
    }

    /** close barrel writer */
    void close();
    /**
    *get collections information of the in-memory index barrel
    */
    CollectionsInfo* getCollectionsInfo()
    {
        return pCollectionsInfo_;
    }
    /**
    *set document schema of all collections, this will initialize different CollectionIndexer, which will proceed relevant document respectively.
    */

    void setCollectionsMeta(const std::map<std::string, IndexerCollectionMeta>& collectionsMeta);

    /**
    *set current indexing mode
    */
    void setIndexMode(bool realtime);

    /**
    *return a handle of CollectionIndexer according to the collection id
    */
    CollectionIndexer* getCollectionIndexer(collectionid_t colID)
    {
        return collectionIndexerMap_[colID];
    }
    /**
    *create an in memory BarrelReader that will read index of this barrel
    */
    IndexBarrelReader* inMemoryReader()
    {
        return new InMemoryIndexBarrelReader(this);
    }

    void setDocFilter(Bitset* pFilter) { pDocFilter_ = pFilter;}

    Bitset* getDocFilter() { return pDocFilter_; }

    void deletebinlog();

private:
    void createMemCache();

    void destroyMemCache();
    /**
     * write cache to barrels
     */
    void flush();
    /**
     * reset cache content
     */
    void reset();

    void flushDocLen();
private:
    BarrelInfo* pBarrelInfo_;

    Indexer* pIndexer_;

    boost::shared_ptr<MemCache> pMemCache_;

    boost::mutex mutex_; /// for flushing MemCache;

    boost::condition_variable cond_;

    CollectionIndexerMap collectionIndexerMap_;

    CollectionsInfo* pCollectionsInfo_;

    Directory* pDirectory_;

    Bitset* pDocFilter_;

    int numFieldIndexers_;

    bool isLastIndexModeRealTime_;

    friend class InMemoryIndexBarrelReader;
    friend class IndexWriter;
};

}

NS_IZENELIB_IR_END

#endif
