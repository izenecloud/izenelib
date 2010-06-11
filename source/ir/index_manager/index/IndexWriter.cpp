#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/OfflineIndexMerger.h>
#include <ir/index_manager/index/ImmediateMerger.h>
#include <ir/index_manager/index/MultiWayMerger.h>
#include <ir/index_manager/index/GPartitionMerger.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>
#include <ir/index_manager/index/IndexMergeManager.h>

#include <util/scheduler.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexWriterWorker::IndexWriterWorker(IndexWriter* pIndexWriter, bool update)
        :pIndexWriter_(pIndexWriter)
        ,update_(update)
        ,pIndexBarrelWriter_(NULL)
        ,pCurBarrelInfo_(NULL)
        ,pIndexMerger_(NULL)
        ,pIndexer_(pIndexWriter->pIndexer_)
        ,pCurDocCount_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();
}

IndexWriterWorker::~IndexWriterWorker()
{
    if (pIndexBarrelWriter_)
        delete pIndexBarrelWriter_;
    if (pIndexMerger_)
        delete pIndexMerger_;
}

void IndexWriterWorker::flush()
{
    if(!pIndexBarrelWriter_)
        return;
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    if (pLastBarrel == NULL)
        return;
    pLastBarrel->setBaseDocID(baseDocIDMap_);
    baseDocIDMap_.clear();
    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        if (pIndexMerger_)
            pIndexMerger_->flushBarrelToDisk(pIndexBarrelWriter_->barrelName_);
    }
    pLastBarrel->setWriter(NULL);
    pBarrelsInfo_->write(pIndexer_->getDirectory());
    delete pIndexBarrelWriter_;
    pIndexBarrelWriter_ = NULL;
}

void IndexWriterWorker::createMerger()
{
    if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"no"))
        pIndexMerger_ = NULL;
    else if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"imm"))
        pIndexMerger_ = new ImmediateMerger(pIndexer_);
    else if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"mway"))
        pIndexMerger_ = new MultiWayMerger(pIndexer_);	
    else
        pIndexMerger_ = new GPartitionMerger(pIndexer_);
}

void IndexWriterWorker::createBarrelWriter()
{
    pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;

    if (!pIndexWriter_->pMemCache_)
        pIndexWriter_->pMemCache_ = 
            new MemCache((size_t)pIndexer_->getIndexManagerConfig()->indexStrategy_.memory_);
    pIndexBarrelWriter_ = new IndexBarrelWriter(pIndexer_,
                                             pIndexWriter_->pMemCache_,pCurBarrelInfo_->getName().c_str());
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pIndexBarrelWriter_->setCollectionsMeta(pIndexer_->getCollectionsMeta());
    if(update_)  pCurBarrelInfo_->isUpdate = true;
}

void IndexWriterWorker::mergeIndex(IndexMerger* pMerger)
{
    pMerger->setDirectory(pIndexer_->getDirectory());

    if(pIndexer_->getIndexReader()->getDocFilter())
        pMerger->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
    ///there is a in-memory index
    if ((pIndexBarrelWriter_) && pCurDocCount_ && ((*pCurDocCount_) > 0))
    {
        IndexMerger* pTmp = pIndexMerger_;
        pIndexMerger_ = pMerger;
        mergeAndWriteCachedIndex();
        pIndexMerger_ = pTmp;
    }
    else
    {
        if(pCurBarrelInfo_)
        {
            //pBarrelsInfo_->deleteLastBarrel();
            pCurBarrelInfo_ = NULL;
            pCurDocCount_ = NULL;
        }
        if(update_)
            pMerger->merge(pBarrelsInfo_, UPDATE_ONLY);
        else
            pMerger->merge(pBarrelsInfo_, INDEX_ONLY);
    }
    pIndexer_->getIndexReader()->delDocFilter();
    delete pIndexMerger_;
    pIndexMerger_ = NULL;
}

void IndexWriterWorker::mergeUpdatedBarrel(docid_t currDocId)
{
    if(!pIndexMerger_) return;
    if(pIndexer_->getIndexReader()->getDocFilter())
        pIndexMerger_->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
    ///there is a in-memory index
    
    if ((pIndexBarrelWriter_) && pCurDocCount_ && ((*pCurDocCount_) > 0))
    {
        mergeAndWriteCachedIndex();

        bool* pHasUpdateDocs =  &(pCurBarrelInfo_->isUpdate);
        *pHasUpdateDocs = true;
    }
    else
        return;

    docid_t lastSetDoc = pIndexer_->getIndexReader()->getDocFilter()->getMaxSet();

    if((currDocId > pBarrelsInfo_->maxDocId())||
            (lastSetDoc > pBarrelsInfo_->maxDocId()))
    {
        bool* pHasUpdateDocs = &(pCurBarrelInfo_->isUpdate);
        *pHasUpdateDocs = false;
    }
    pIndexer_->getIndexReader()->delDocFilter();	
    pIndexMerger_->setDocFilter(NULL);
}

void IndexWriterWorker::mergeAndWriteCachedIndex()
{
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    pLastBarrel->setBaseDocID(baseDocIDMap_);

    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        pLastBarrel->setWriter(NULL);
        pBarrelsInfo_->write(pIndexer_->getDirectory());
    }
    if(update_)
        pIndexMerger_->merge(pBarrelsInfo_, UPDATE_ONLY);
    else
        pIndexMerger_->merge(pBarrelsInfo_, INDEX_ONLY);

    pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;
}

void IndexWriterWorker::addToMergeAndWriteCachedIndex()
{
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    pLastBarrel->setBaseDocID(baseDocIDMap_);

    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        pLastBarrel->setWriter(NULL);
        pBarrelsInfo_->write(pIndexer_->getDirectory());
    }

    if (pIndexMerger_)
        pIndexMerger_->addToMerge(pBarrelsInfo_,pBarrelsInfo_->getLastBarrel());
	
    if (pIndexMerger_)
        pIndexMerger_->flushBarrelToDisk(pIndexBarrelWriter_->barrelName_);

    pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;
}

void IndexWriterWorker::writeCachedIndex()
{
    pBarrelsInfo_->wait_for_barrels_ready();
    pCurBarrelInfo_->setBaseDocID(baseDocIDMap_);
    if (pIndexBarrelWriter_->cacheEmpty() == false)
    {
        pIndexBarrelWriter_->close();
        pCurBarrelInfo_->setWriter(NULL);
        pBarrelsInfo_->write(pIndexer_->getDirectory());
    }
    pIndexWriter_->pIndexMergeManager_->triggerMerge(pCurBarrelInfo_,update_);
    pBarrelsInfo_->setLock(true);
    if(pIndexer_->getIndexerType()&MANAGER_INDEXING_STANDALONE_MERGER)
    {
        pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    }
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurBarrelInfo_->setSearchable(true);
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pBarrelsInfo_->setLock(false);

    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;
}

void IndexWriterWorker::addDocument(IndexerDocument& doc)
{
    if(!pIndexBarrelWriter_) createBarrelWriter();
    if(!pIndexMerger_) createMerger();

    DocId uniqueID;
    doc.getDocId(uniqueID);

    if(update_)
    {
        if(uniqueID.docId > pBarrelsInfo_->maxDocId())
            return;
        pIndexer_->getIndexReader()->delDocument(uniqueID.colId,uniqueID.docId);
    }

    if (baseDocIDMap_.find(uniqueID.colId) == baseDocIDMap_.end())
        baseDocIDMap_.insert(make_pair(uniqueID.colId,uniqueID.docId));

    if (pIndexBarrelWriter_->cacheFull())
    {
        if(pIndexer_->getIndexerType()&MANAGER_INDEXING_STANDALONE_MERGER)
            writeCachedIndex();
        else
            ///merge index
            addToMergeAndWriteCachedIndex();
        baseDocIDMap_.clear();
        baseDocIDMap_[uniqueID.colId] = uniqueID.docId;
        pIndexBarrelWriter_->open(pCurBarrelInfo_->getName().c_str());
    }
    pCurBarrelInfo_->updateMaxDoc(uniqueID.docId);
    pBarrelsInfo_->updateMaxDoc(uniqueID.docId);
    pIndexBarrelWriter_->addDocument(doc);
    (*pCurDocCount_)++;
}

void IndexWriterWorker::removeDocument(collectionid_t colID, docid_t docId)
{
    pIndexer_->getIndexReader()->delDocument(colID, docId);
    if(! pIndexBarrelWriter_->getDocFilter())
        pIndexBarrelWriter_->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
}

void IndexWriterWorker::optimizeIndex()
{
    IndexMerger* pIndexMerger = new OfflineIndexMerger(pIndexer_, pBarrelsInfo_->getBarrelCount());
    mergeIndex(pIndexMerger);
    delete pIndexMerger;
}

IndexWriter::IndexWriter(Indexer* pIndex)
        :pMemCache_(NULL)
        ,pIndexer_(pIndex)
        ,pIndexMergeManager_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();
    pIndexWorker_ = new IndexWriterWorker(this, true);
    pUpdateWorker_ = new IndexWriterWorker(this, false);
    pIndexMergeManager_ = new IndexMergeManager(pIndex);
    pIndexMergeManager_->run();
}

IndexWriter::~IndexWriter()
{
    if (pMemCache_)
        delete pMemCache_;
    if (pIndexWorker_)
        delete pIndexWorker_;
    if (pUpdateWorker_)
        delete pUpdateWorker_;
    if (pIndexMergeManager_)
        delete pIndexMergeManager_;
}

void IndexWriter::indexDocument(IndexerDocument& doc)
{
    pIndexWorker_->addDocument(doc);
}

void IndexWriter::removeDocument(collectionid_t colID, docid_t docId)
{
    pIndexWorker_->removeDocument(colID, docId);
}

void IndexWriter::updateDocument(IndexerDocument& doc)
{
    pUpdateWorker_->addDocument(doc);
}

void IndexWriter::flush()
{
    pIndexWorker_->flush();
    pUpdateWorker_->flush();
}

void IndexWriter::optimizeIndex()
{
    if(pIndexer_->getIndexerType()&MANAGER_INDEXING_STANDALONE_MERGER)
    {
        ///optimize asynchronously
        pIndexMergeManager_->optimizeIndex();
    }
    else
    {
        ///optimize synchronously
        pIndexWorker_->optimizeIndex();
        pUpdateWorker_->optimizeIndex();
        IndexMerger* pIndexMerger = new OfflineIndexMerger(pIndexer_, pBarrelsInfo_->getBarrelCount());
        pIndexMerger->setDirectory(pIndexer_->getDirectory());
        if(pIndexer_->getIndexReader()->getDocFilter())
            pIndexMerger->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
        pIndexMerger->merge(pBarrelsInfo_);
        pIndexer_->getIndexReader()->delDocFilter();
        delete pIndexMerger;
    }
}

void IndexWriter::lazyOptimizeIndex()
{
    using namespace boost::posix_time;
    using namespace boost::gregorian;
    using namespace izenelib::util;

    boost::posix_time::ptime now = second_clock::local_time();
    boost::gregorian::date date = now.date();
    boost::posix_time::time_duration du = now.time_of_day();
    int dow = date.day_of_week();
    int month = date.month();
    int day = date.day();
    int hour = du.hours();
    int minute = du.minutes();
    if(scheduleExpression_.matches(minute, hour, day, month, dow))
    {
        pIndexMergeManager_->optimizeIndex();
    }
}

void IndexWriter::scheduleOptimizeTask(std::string expression, string uuid)
{
    scheduleExpression_.setExpression(expression);

    const string optimizeJob = "optimizeindex"+uuid;
    ///we need an uuid here because scheduler is an singleton in the system
    Scheduler::removeJob(optimizeJob);
    
    boost::function<void (void)> task = boost::bind(&IndexWriter::lazyOptimizeIndex,this);
    Scheduler::addJob(optimizeJob, 60*1000, 0, task);
}

}

NS_IZENELIB_IR_END
