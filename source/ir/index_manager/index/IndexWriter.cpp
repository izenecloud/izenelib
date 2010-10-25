#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/OfflineIndexMerger.h>
#include <ir/index_manager/index/MultiWayMerger.h>
#include <ir/index_manager/index/GPartitionMerger.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>
#include <ir/index_manager/index/IndexMergeManager.h>

#include <util/scheduler.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexWriter::IndexWriter(Indexer* pIndex)
        :pMemCache_(NULL)
        ,pIndexer_(pIndex)
        ,pIndexBarrelWriter_(NULL)
        ,pBarrelsInfo_(NULL)
        ,pCurBarrelInfo_(NULL)
        ,pIndexMerger_(NULL)
        ,pIndexMergeManager_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();
    pMemCache_ = new MemCache((size_t)pIndexer_->getIndexManagerConfig()->indexStrategy_.memory_);
    pIndexBarrelWriter_ = new IndexBarrelWriter(pIndexer_, pMemCache_);
    pIndexBarrelWriter_->setCollectionsMeta(pIndexer_->getCollectionsMeta());

    if(pIndexer_->getIndexerType()&MANAGER_INDEXING_STANDALONE_MERGER)
    {
        pIndexMergeManager_ = new IndexMergeManager(pIndex);
        pIndexMergeManager_->run();
    }
}

IndexWriter::~IndexWriter()
{
    if (pMemCache_)
        delete pMemCache_;
    if (pIndexBarrelWriter_)
        delete pIndexBarrelWriter_;
    if (pIndexMerger_)
        delete pIndexMerger_;
}

void IndexWriter::close()
{
    if (pIndexMergeManager_)
    {
        pIndexMergeManager_->stop();
        //delete pIndexMergeManager_;
        //pIndexMergeManager_ = NULL;
    }
}

void IndexWriter::flush()
{
    DVLOG(2) << "=> IndexWriter::flush()...";
    if(pCurBarrelInfo_ == NULL)
        return;

    if(pIndexer_->isRealTime())
    {
        if(pIndexBarrelWriter_->cacheEmpty() == false)
        {
            ///memory index has not been written to database yet.
            if(pIndexer_->getIndexerType()&MANAGER_INDEXING_STANDALONE_MERGER)
            {
                pBarrelsInfo_->wait_for_barrels_ready();
                pIndexBarrelWriter_->close();
                pIndexMergeManager_->triggerMerge(pCurBarrelInfo_);
            }
            else
            {
                pIndexBarrelWriter_->close();
                if (pIndexMerger_)
                    pIndexMerger_->flushBarrelToDisk(pCurBarrelInfo_->getName());
            }
        }
    }
    else
    {
        try
        {
            pIndexBarrelWriter_->writeCache();
            pCurBarrelInfo_->setSearchable(true);
        }catch(std::exception& e)
        {
            cerr << e.what() << endl;
        }
    }

    pBarrelsInfo_->write(pIndexer_->getDirectory());
    pCurBarrelInfo_ = NULL;
    DVLOG(2) << "<= IndexWriter::flush()";
}

void IndexWriter::createMerger()
{
    if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"no"))
        pIndexMerger_ = NULL;
    else if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"mway"))
        pIndexMerger_ = new MultiWayMerger(pIndexer_);	
    else
        pIndexMerger_ = new GPartitionMerger(pIndexer_);
}

void IndexWriter::createMemCache()
{
    if (!pMemCache_)
        pMemCache_ = 
            new MemCache((size_t)pIndexer_->getIndexManagerConfig()->indexStrategy_.memory_);
}

void IndexWriter::createBarrelInfo()
{
    DVLOG(2) << "=> IndexWriter::createBarrelInfo()...";

    pCurBarrelInfo_ = new BarrelInfo(pBarrelsInfo_->newBarrel(),0);
    pCurBarrelInfo_->setSearchable(pIndexer_->isRealTime());
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pIndexBarrelWriter_->open(pCurBarrelInfo_);
    pBarrelsInfo_->addBarrel(pCurBarrelInfo_,false);

    DVLOG(2) << "<= IndexWriter::createBarrelInfo()";
}

void IndexWriter::mergeIndex(IndexMerger* pMerger)
{
    pMerger->setDirectory(pIndexer_->getDirectory());

    if(pIndexer_->getIndexReader()->getDocFilter())
        pMerger->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
    ///there is a in-memory index
    if (pCurBarrelInfo_ && pCurBarrelInfo_->nNumDocs > 0)
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
        }
        pMerger->merge(pBarrelsInfo_);
    }
    pIndexer_->getIndexReader()->delDocFilter();
    delete pIndexMerger_;
    pIndexMerger_ = NULL;
}

void IndexWriter::mergeAndWriteCachedIndex()
{
    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        pBarrelsInfo_->write(pIndexer_->getDirectory());
    }
    pIndexMerger_->merge(pBarrelsInfo_);

    createBarrelInfo();
}

void IndexWriter::addToMergeAndWriteCachedIndex()
{
    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        pBarrelsInfo_->write(pIndexer_->getDirectory());
    }

    if (pIndexMerger_)
        pIndexMerger_->addToMerge(pBarrelsInfo_,pCurBarrelInfo_);
	
    if (pIndexMerger_)
        pIndexMerger_->flushBarrelToDisk(pCurBarrelInfo_->getName());

    createBarrelInfo();
}

void IndexWriter::writeCachedIndex()
{
    pBarrelsInfo_->wait_for_barrels_ready();
    if (pIndexBarrelWriter_->cacheEmpty() == false)
    {
        pIndexBarrelWriter_->close();
        pBarrelsInfo_->write(pIndexer_->getDirectory());
    }
	
    pIndexMergeManager_->triggerMerge(pCurBarrelInfo_);

    pBarrelsInfo_->setLock(true);

    createBarrelInfo();

    pBarrelsInfo_->setLock(false);
}

void IndexWriter::indexDocument(IndexerDocument& doc)
{
    if(!pCurBarrelInfo_) createBarrelInfo();
    if(!pIndexMerger_) createMerger();

    if(pIndexer_->isRealTime())
    {
        if (pIndexBarrelWriter_->cacheFull())
        {
            if(pIndexer_->getIndexerType()&MANAGER_INDEXING_STANDALONE_MERGER)
                writeCachedIndex();
            else
                ///merge index
                addToMergeAndWriteCachedIndex();
        }
    }

    DocId uniqueID;
    doc.getDocId(uniqueID);

    if(pCurBarrelInfo_->getBaseDocID() == BAD_DOCID)
        pCurBarrelInfo_->addBaseDocID(uniqueID.colId,uniqueID.docId);

    pCurBarrelInfo_->updateMaxDoc(uniqueID.docId);
    pBarrelsInfo_->updateMaxDoc(uniqueID.docId);
    ++(pCurBarrelInfo_->nNumDocs);
    pIndexBarrelWriter_->addDocument(doc);
}

void IndexWriter::removeDocument(collectionid_t colID, docid_t docId)
{
    pIndexer_->getIndexReader()->delDocument(colID, docId);
    if(! pIndexBarrelWriter_->getDocFilter())
        pIndexBarrelWriter_->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
}

void IndexWriter::updateDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

    if(doc.getId() > pBarrelsInfo_->maxDocId())
        return;
    indexDocument(doc);
    removeDocument(uniqueID.colId,doc.getId());
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
