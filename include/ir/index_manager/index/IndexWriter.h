/**
* @file        IndexWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Indexing the documents
*/
#ifndef INDEXWRITER_H
#define INDEXWRITER_H

#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/utility/MemCache.h>

#include <util/cronexpression.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Indexer;
class IndexMerger;
class IndexMergeManager;
/**
* @brief IndexWriter is the manager class which process the index construction and index merging
*/
class IndexWriter
{
public:
    IndexWriter(Indexer* pIndex);

    ~IndexWriter();
public:
    /// add a document to the internal cache, if the cache is full, then index all the cached documents.
    void addDocument(IndexerDocument* pDoc, bool update = false);
    /// index the document object practically
    void indexDocument(IndexerDocument* pDoc, bool update = false);
    ///merge the barrels index manually using an existing IndexMerger
    void mergeIndex(IndexMerger* pIndexMerger);

    void optimizeIndex();

    void flush();

    void close();

    void scheduleOptimizeTask(std::string expression, string uuid);
private:

    void setupCache();

    void destroyCache();

    void clearCache();

    bool isCacheFull() { return nNumCacheUsed_ >= nNumCachedDocs_; }

    void lazyOptimizeIndex();

   ///IndexBarrelWriter is the practical class to process indexing procedure.
    void createBarrelWriter(bool update = false);

    void createMerger();
    ///will be used by mergeIndex
    void mergeAndWriteCachedIndex(bool mergeUpdateOnly = false);
    ///will be used when index documents
    void addToMergeAndWriteCachedIndex();

    void writeCachedIndex();	
    ///merge the updated barrel
    void mergeUpdatedBarrel(docid_t currDocId);
    ///when the memory cache of IndexWriter is full, then index all the cached index. 
    ///Call this function directly will force to index all the cached docuement objects.
    void flushDocuments(bool update = false);
private:
    IndexBarrelWriter* pIndexBarrelWriter_;

    map<collectionid_t,docid_t> baseDocIDMap_;

    IndexerDocument** ppCachedDocs_;

    int nNumCachedDocs_;

    int nNumCacheUsed_;

    BarrelsInfo* pBarrelsInfo_;

    BarrelInfo* pCurBarrelInfo_;

    IndexMerger* pIndexMerger_;

    MemCache* pMemCache_;

    Indexer* pIndexer_;

    count_t* pCurDocCount_;

    IndexMergeManager* pIndexMergeManager_;

    izenelib::util::CronExpression scheduleExpression_;
};

}

NS_IZENELIB_IR_END

#endif
