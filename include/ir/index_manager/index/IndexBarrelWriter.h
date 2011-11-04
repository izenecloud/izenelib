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
#include <ir/index_manager/index/BTreeIndex.h>

#include <ir/index_manager/store/Directory.h>

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/utility/BitVector.h>
#include <util/izene_log.h>

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
     * add a analyzed IndexerDocument
     * @param doc analyzed document
     */
    void addDocument(IndexerDocument& doc);

    /**
     * update IndexerDocument
     * @param oldDoc the old document
     * @param doc the new document
     */
    void updateDocument(IndexerDocument& oldDoc, IndexerDocument& doc);
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

    void setDocFilter(BitVector* pFilter) { pDocFilter_ = pFilter;}

    BitVector* getDocFilter() { return pDocFilter_; }

    bool isDirty() { return dirty_; }

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

private:
    BarrelInfo* pBarrelInfo_;

    Indexer* pIndexer_;

    MemCache* pMemCache_;

    CollectionIndexerMap collectionIndexerMap_;

    CollectionsInfo* pCollectionsInfo_;

    Directory* pDirectory_;

    BitVector* pDocFilter_;

    volatile bool dirty_;

    friend class InMemoryIndexBarrelReader;
};

}

NS_IZENELIB_IR_END

#endif
