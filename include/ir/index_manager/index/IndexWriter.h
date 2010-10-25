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
#include <string>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Indexer;
class IndexMerger;
class IndexMergeManager;
/**
* @brief IndexWriter is the manager class which process the index construction and index merging
* IndexWriter does not support concurrent index construction, because it will lead to many unnecessary 
* complexity. So we can not index two documents at one time, also, we can not index one document 
* and update another document at one time,either.
*/
class IndexWriter
{
public:
    IndexWriter(Indexer* pIndex);

    ~IndexWriter();
public:
    /// index the document object
    void indexDocument(IndexerDocument& doc);
    /// remove the document
    void removeDocument(collectionid_t colID, docid_t docId);
    /// update the document object
    void updateDocument(IndexerDocument& doc);
     /// optimize index
    void optimizeIndex();
    /// flush index
    void flush();
    /// set schedule 
    void scheduleOptimizeTask(std::string expression, string uuid);
    /// close
    void close();

    void createMemCache();

private:
    void createBarrelInfo();
     /// create index merger
    void createMerger();
    /// mergeIndex
    void mergeIndex(IndexMerger * pMerger);
    ///will be used by mergeIndex
    void mergeAndWriteCachedIndex();
    ///will be used when index documents
    void addToMergeAndWriteCachedIndex();
    /// flush in-memory index to disk
    void writeCachedIndex();	
    /// optimize index offline
    void lazyOptimizeIndex();
private:
    MemCache* pMemCache_;

    Indexer* pIndexer_;

    IndexBarrelWriter* pIndexBarrelWriter_;

    BarrelsInfo* pBarrelsInfo_;

    BarrelInfo* pCurBarrelInfo_;

    IndexMerger* pIndexMerger_;

    IndexMergeManager* pIndexMergeManager_;

    izenelib::util::CronExpression scheduleExpression_;

    friend class IndexMerger;
};

}

NS_IZENELIB_IR_END

#endif
