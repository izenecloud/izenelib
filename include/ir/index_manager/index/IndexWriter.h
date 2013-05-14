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
    /// update not R-type document object
    void updateDocument(IndexerDocument& doc);
    /// update R-type document object
    void updateRtypeDocument(IndexerDocument& oldDoc, IndexerDocument& doc);
    /// optimize index
    void optimizeIndex();
    /// close
    void close();
    /// flush index
    void flush();
    /// flush barrels info 
    void flushBarrelsInfo();
    /// flush doclen writer
    void flushDocLen();
    /// set schedule
    //void scheduleOptimizeTask(std::string expression, string uuid);

    IndexMergeManager* getMergeManager() { return pIndexMergeManager_; }

    ///set current indexing mode
    void setIndexMode(bool realtime);

    void tryResumeExistingBarrels();

    BarrelInfo* getBarrelInfo() { return pCurBarrelInfo_; }

    BarrelsInfo* getBarrelsInfo() { return pBarrelsInfo_ ;}

    IndexBarrelWriter* getIndexBarrelWriter() { return pIndexBarrelWriter_; }

    void createBarrelInfo();

    void checkbinlog();

    void deletebinlog();

private:
    
    /// optimize index offline
    //void lazyOptimizeIndex(int calltype);
private:
    Indexer* pIndexer_;

    IndexBarrelWriter* pIndexBarrelWriter_;

    BarrelsInfo* pBarrelsInfo_;

    BarrelInfo* pCurBarrelInfo_;

    IndexMergeManager* pIndexMergeManager_;

    //izenelib::util::CronExpression scheduleExpression_;

    //std::string optimizeJobDesc_;

    boost::mutex indexMutex_;

    friend class IndexMerger;
};

}

NS_IZENELIB_IR_END

#endif
