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
class IndexWriter;
class IndexMerger;
class IndexMergeManager;
/*
* Helper classs to proceed the index construction
*/
class IndexWriterWorker
{
public:
    IndexWriterWorker(IndexWriter* pIndexWriter, bool update);

    ~IndexWriterWorker();
public:
    /// index document
    void addDocument(IndexerDocument& doc);
    /// remove document
    void removeDocument(collectionid_t colID, docid_t docId);
     /// merge the barrels index manually using an existing IndexMerger
    void mergeIndex(IndexMerger* pIndexMerger);
    /// optimize index
    void optimizeIndex();
    /// flush index
    void flush();
private:
    void createBarrelWriter();
     /// create index merger
    void createMerger();
    ///will be used by mergeIndex
    void mergeAndWriteCachedIndex();
    ///will be used when index documents
    void addToMergeAndWriteCachedIndex();
    /// flush in-memory index to disk
    void writeCachedIndex();	
    ///merge the updated barrel
    void mergeUpdatedBarrel(docid_t currDocId);
private:
    IndexWriter* pIndexWriter_;

    bool update_; /// true: merge update barrels only  false: merge index barrels only

    IndexBarrelWriter* pIndexBarrelWriter_;

    map<collectionid_t,docid_t> baseDocIDMap_;

    BarrelsInfo* pBarrelsInfo_;

    BarrelInfo* pCurBarrelInfo_;

    IndexMerger* pIndexMerger_;

    Indexer* pIndexer_;

    count_t* pCurDocCount_;
};

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
private:
    /// optimize index offline
    void lazyOptimizeIndex();
    /// IndexBarrelWriter is the practical class to process indexing procedure.
private:
    IndexWriterWorker* pIndexWorker_;

    IndexWriterWorker* pUpdateWorker_;

    MemCache* pMemCache_;

    Indexer* pIndexer_;

    BarrelsInfo* pBarrelsInfo_;

    IndexMergeManager* pIndexMergeManager_;

    izenelib::util::CronExpression scheduleExpression_;

    friend class IndexWriterWorker;
};

}

NS_IZENELIB_IR_END

#endif
