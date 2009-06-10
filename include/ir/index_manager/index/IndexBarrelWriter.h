/**
* @file        IndexBarrelWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index the documents into barrels
*/
#ifndef INDEXBARRELWRITER_H
#define INDEXBARRELWRITER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/utility/Logger.h>
#include <ir/index_manager/store/Directory.h>

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexerCollectionMeta.h>
#include <ir/index_manager/index/CollectionIndexer.h>
#include <ir/index_manager/index/CollectionInfo.h>
#include <ir/index_manager/index/InMemoryIndexBarrelReader.h>
#include <ir/index_manager/index/BTreeIndex.h>

#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef map<collectionid_t, CollectionIndexer*> CollectionIndexerMap;
class Indexer;
class IndexBarrelWriter
{
public:
    IndexBarrelWriter(Indexer* pIndexer,MemCache* pCache,const char* name);

    ~IndexBarrelWriter();
public:
    /**
     * add a analyzed IndexerDocument
     * @param pDoc analyzed document
     */
    void addDocument(IndexerDocument* pDoc);
    /**
     * delete a IndexerDocument in a barrel
     * @param pDoc analyzed document
     * @param inMemory whether this barrel is in memory or not
     */
    bool deleteDocument(IndexerDocument* pDoc, bool inMemory);
    /**
     * determine if the memory cache for indexing is full
     * @return true if cache is full otherwise false.
     */
    bool cacheFull()
    {
        return pMemCache->isGrowed();
    }
    /**
     * determine if the memory cache for indexing is empty
     * @return true if empty,otherwise false
     */
    bool cacheEmpty()
    {
        return pMemCache->isEmpty();
    }
    /**
     * write cache to barrels
     */
    void writeCache();
    /**
     * reset cache content
     * @param bResetPosting reset postings of indexer or not
     */
    void resetCache(bool bResetPosting = false);
    /**
     * open a index barrel
     * @param barrelName index barrel name
    */
    void open(const char* barrelName);
    /** close barrel writer */
    void close();
    /**
    * set a new name of the barrel
    * @param newName new name of the barrel
    */
    void rename(const char* newName);

    const string& getBarrelName()
    {
        return barrelName;
    }
    /**
    *get collections information of the in-memory index barrel
    */
    CollectionsInfo* getCollectionsInfo()
    {
        return pCollectionsInfo;
    }
    /**
    *set document schema of all collections, this will initialize different CollectionIndexer, which will proceed relevant document respectively.
    */

    void setCollectionsMeta(const std::map<std::string, IndexerCollectionMeta>& collectionsMeta);

    /**
    *return a handle of CollectionIndexer according to the collection id
    */
    CollectionIndexer* getCollectionIndexer(collectionid_t colID)
    {
        return collectionIndexMap[colID];
    }
    /**
    *create an in memory BarrelReader that will read index of this barrel
    */
    IndexBarrelReader* inMemoryReader()
    {
        return new InMemoryIndexBarrelReader(this);
    }
    /**
    * get memory cache for indexing
    */
    MemCache* getMemCache()
    {
        return pMemCache;
    }
    /**
     * set memory cache for indexing
     */
    void setMemCache(MemCache* pMemCache)
    {
        this->pMemCache = pMemCache;
    }

private:
    string barrelName;

    Indexer* pIndexer;

    MemCache* pMemCache;

    CollectionIndexerMap collectionIndexMap;

    CollectionsInfo* pCollectionsInfo;

    collectionid_t currentColID;

    Directory* pDirectory;

    friend class InMemoryIndexBarrelReader;
    friend class IndexWriter;
};

}

NS_IZENELIB_IR_END

#endif
