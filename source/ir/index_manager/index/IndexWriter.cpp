#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/thread.hpp>

#include <cassert>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>
#include <ir/index_manager/index/IndexMergeManager.h>
#include <ir/index_manager/index/rtype/BTreeIndexerManager.h>

#include <util/scheduler.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexWriter::IndexWriter(Indexer* pIndex)
        :pIndexer_(pIndex)
        ,pIndexBarrelWriter_(NULL)
        ,pBarrelsInfo_(NULL)
        ,pCurBarrelInfo_(NULL)
        ,pIndexMergeManager_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    pIndexMergeManager_ = new IndexMergeManager(pIndex);

}

IndexWriter::~IndexWriter()
{
    if(!optimizeJobDesc_.empty())
        Scheduler::removeJob(optimizeJobDesc_);
    if (pIndexMergeManager_)
        delete pIndexMergeManager_;
    if (pIndexBarrelWriter_)
        delete pIndexBarrelWriter_;
}

void IndexWriter::tryResumeExistingBarrels()
{
    // in order to resume merging previous barrels,
    // add them into pIndexMergeManager_
    BarrelInfo* pBarrelInfo = NULL;
    boost::mutex::scoped_lock lock(pBarrelsInfo_->getMutex());
    pBarrelsInfo_->startIterator();
    while(pBarrelsInfo_->hasNext())
    {
        pBarrelInfo = pBarrelsInfo_->next();
        assert(pBarrelInfo->getWriter() == NULL && "the loaded BarrelInfo should not be in-memory barrel");
        pIndexMergeManager_->addToMerge(pBarrelInfo);
    }
}

void IndexWriter::close()
{
    ///The difference between close and flush is close don't need t reopen indexreader
    DVLOG(2) << "=> IndexWriter::close()...";
    if(pCurBarrelInfo_ == NULL)
    {
        // write file "barrels" to update the doc count of each barrel
        if(pBarrelsInfo_->getBarrelCount() > 0)
            pBarrelsInfo_->write(pIndexer_->getDirectory());
        DVLOG(2) << "<= IndexWriter::flush(), pCurBarrelInfo_ is NULL";
        return;
    }
    assert(pIndexBarrelWriter_ && "pIndexBarrelWriter_ should have been created with pCurBarrelInfo_ together in IndexWriter::createBarrelInfo()");

    pIndexBarrelWriter_->flush();
    pBarrelsInfo_->write(pIndexer_->getDirectory());
    pIndexBarrelWriter_->reset();
    pCurBarrelInfo_ = NULL;
    DVLOG(2) << "<= IndexWriter::close()";
}

void IndexWriter::flush()
{
    DVLOG(2) << "=> IndexWriter::flush()...";
    if(pCurBarrelInfo_ == NULL)
    {
        // write file "barrels" to update the doc count of each barrel
        if(pBarrelsInfo_->getBarrelCount() > 0)
            pBarrelsInfo_->write(pIndexer_->getDirectory());
        DVLOG(2) << "<= IndexWriter::flush(), pCurBarrelInfo_ is NULL";
        return;
    }
    assert(pIndexBarrelWriter_ && "pIndexBarrelWriter_ should have been created with pCurBarrelInfo_ together in IndexWriter::createBarrelInfo()");

    pIndexBarrelWriter_->flush();
    if(! pIndexer_->isRealTime())
        pCurBarrelInfo_->setSearchable(true);
    pIndexer_->setDirty();
    pIndexer_->getIndexReader();

    pIndexMergeManager_->addToMerge(pCurBarrelInfo_);

    pBarrelsInfo_->write(pIndexer_->getDirectory());

    pIndexBarrelWriter_->reset();
    pCurBarrelInfo_ = NULL;
    DVLOG(2) << "<= IndexWriter::flush()";
}

void IndexWriter::flushDocLen() 
{
    if(pIndexBarrelWriter_) pIndexBarrelWriter_->flushDocLen(); 
}

void IndexWriter::createBarrelInfo()
{
   DVLOG(2)<< "=> IndexWriter::createBarrelInfo()...";

    pCurBarrelInfo_ = new BarrelInfo(pBarrelsInfo_->newBarrel(), 0, pIndexer_->pConfigurationManager_->indexStrategy_.indexLevel_, pIndexer_->getIndexCompressType());
    pCurBarrelInfo_->setSearchable(pIndexer_->isRealTime());

    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pIndexBarrelWriter_->setBarrelInfo(pCurBarrelInfo_);
    pBarrelsInfo_->addBarrel(pCurBarrelInfo_);

    DVLOG(2)<< "<= IndexWriter::createBarrelInfo()";
}

void IndexWriter::indexDocument(IndexerDocument& doc)
{
    boost::lock_guard<boost::mutex> lock(indexMutex_);

    if(!pCurBarrelInfo_) createBarrelInfo();

    if(pIndexer_->isRealTime())
    {
        ///If indexreader has not contained the in-memory barrel reader,
        ///The dirty flag should be set, so that the new query could open the in-memory barrel
        ///for real time query
        if(!pIndexer_->pIndexReader_->hasMemBarrelReader())
            pIndexer_->setDirty();
        if(pIndexBarrelWriter_->cacheFull())
        {
            DVLOG(2) << "IndexWriter::indexDocument() => realtime cache full...";
            flush();
            createBarrelInfo();
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
    pIndexer_->pBTreeIndexer_->delDocument(pIndexer_->getBarrelsInfo()->maxDocId() + 1, docId);
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

void IndexWriter::updateRtypeDocument(IndexerDocument& oldDoc, IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

    std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >& 
        propertyValueList = doc.getPropertyList();
    std::map<IndexerPropertyConfig, IndexerDocumentPropertyType> oldDocMap;
    oldDoc.to_map(oldDocMap);
    for (std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >::iterator iter
                = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (iter->first.isIndex() && iter->first.isFilter())
        {
            map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator it;
            if( (it = oldDocMap.find(iter->first)) != oldDocMap.end() )
            {
                if(it->first.isMultiValue())
                {
                    MultiValuePropertyType oldProp;
                    oldProp = boost::get<MultiValuePropertyType>(it->second);
                    for(MultiValuePropertyType::iterator multiIt = oldProp.begin(); multiIt != oldProp.end(); ++multiIt)
                       pIndexer_->getBTreeIndexer()->remove(iter->first.getName(), *multiIt, uniqueID.docId);
                }
                else
                {
                    PropertyType oldProp;
                    oldProp = boost::get<PropertyType>(it->second);
                    pIndexer_->getBTreeIndexer()->remove(iter->first.getName(), oldProp, uniqueID.docId);
                }
            }
            if(iter->first.isMultiValue())
            {
                MultiValuePropertyType prop;
                prop = boost::get<MultiValuePropertyType>(iter->second);
                for(MultiValuePropertyType::iterator multiIt = prop.begin(); multiIt != prop.end(); ++multiIt)
                    pIndexer_->getBTreeIndexer()->add(iter->first.getName(), *multiIt, uniqueID.docId);
            }
            else
            {
                PropertyType prop;
                prop = boost::get<PropertyType>(iter->second);
                pIndexer_->getBTreeIndexer()->add(iter->first.getName(), prop, uniqueID.docId);
            }
        //}
        }
    }
}

void IndexWriter::optimizeIndex()
{
    boost::lock_guard<boost::mutex> lock(indexMutex_);
    flush();
    pIndexMergeManager_->optimizeIndex();
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
        boost::lock_guard<boost::mutex> lock(indexMutex_);
        flush();
        pIndexMergeManager_->optimizeIndex();
    }
}

void IndexWriter::scheduleOptimizeTask(std::string expression, string uuid)
{
    scheduleExpression_.setExpression(expression);

    const string optimizeJob = "optimizeindex"+uuid;
    ///we need an uuid here because scheduler is an singleton in the system
    Scheduler::removeJob(optimizeJob);

    optimizeJobDesc_ = optimizeJob;
    boost::function<void (void)> task = boost::bind(&IndexWriter::lazyOptimizeIndex,this);
    Scheduler::addJob(optimizeJob, 60*1000, 0, task);
}

void IndexWriter::setIndexMode(bool realtime)
{
    if(!pIndexBarrelWriter_)
    {
        pIndexBarrelWriter_ = new IndexBarrelWriter(pIndexer_);
        pIndexBarrelWriter_->setCollectionsMeta(pIndexer_->getCollectionsMeta());
    }

    pIndexBarrelWriter_->setIndexMode(realtime);
}

}

NS_IZENELIB_IR_END
