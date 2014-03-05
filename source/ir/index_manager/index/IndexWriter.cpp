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
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <util/scheduler.h>
#include <fstream>
using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
namespace bfs = boost::filesystem;
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
    //if (!optimizeJobDesc_.empty())
    //    Scheduler::removeJob(optimizeJobDesc_);
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
    BarrelInfo* pBinlogInfo = NULL;
    boost::mutex::scoped_lock lock(pBarrelsInfo_->getMutex());
    pBarrelsInfo_->startIterator();
    while (pBarrelsInfo_->hasNext())
    {
        pBarrelInfo = pBarrelsInfo_->next();
        if (pBarrelInfo->isRealTime())
        {
            pBinlogInfo = pBarrelInfo;
            continue;
        }
        assert(pBarrelInfo->getWriter() == NULL && "the loaded BarrelInfo should not be in-memory barrel");
        pIndexMergeManager_->addToMerge(pBarrelInfo);
    }

    if (pBinlogInfo)
    {
        pCurBarrelInfo_ = pBinlogInfo;
        pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
        pIndexBarrelWriter_->setBarrelInfo(pCurBarrelInfo_);
    }
}

void IndexWriter::close()
{
    ///The difference between close and flush is close don't need t reopen indexreader
    DVLOG(2) << "=> IndexWriter::close()...";
    if (pCurBarrelInfo_ == NULL)
    {
        // write file "barrels" to update the doc count of each barrel
        if (pBarrelsInfo_->getBarrelCount() > 0)
            pBarrelsInfo_->write(pIndexer_->getDirectory());
        DVLOG(2) << "<= IndexWriter::flush(), pCurBarrelInfo_ is NULL";
        return;
    }
    assert(pIndexBarrelWriter_ && "pIndexBarrelWriter_ should have been created with pCurBarrelInfo_ together in IndexWriter::createBarrelInfo()");
    /*for realtime index need not flush when stop normally*/
    //pIndexBarrelWriter_->flush();
    //pBarrelsInfo_->write(pIndexer_->getDirectory());

    pIndexBarrelWriter_->reset();
    pCurBarrelInfo_ = NULL;
    DVLOG(2) << "<= IndexWriter::close()";

}

void IndexWriter::flush()
{
    DVLOG(2) << "=> IndexWriter::flush()...";
    if (!pCurBarrelInfo_)
    {
        // write file "barrels" to update the doc count of each barrel
        if (pBarrelsInfo_->getBarrelCount() > 0)
            pBarrelsInfo_->write(pIndexer_->getDirectory());
        DVLOG(2) << "<= IndexWriter::flush(), pCurBarrelInfo_ is NULL";
        return;
    }
    assert(pIndexBarrelWriter_ && "pIndexBarrelWriter_ should have been created with pCurBarrelInfo_ together in IndexWriter::createBarrelInfo()");

    pIndexBarrelWriter_->flush();
    if (!pIndexer_->isRealTime())
        pCurBarrelInfo_->setSearchable(true);
    pIndexer_->setDirty();
    pIndexer_->getIndexReader();

    pIndexMergeManager_->addToMerge(pCurBarrelInfo_);

    pBarrelsInfo_->write(pIndexer_->getDirectory());

    pIndexBarrelWriter_->reset();
    pCurBarrelInfo_ = NULL;
    DVLOG(2) << "<= IndexWriter::flush()";
}

/* change for distribute sf1. */
void IndexWriter::flushBarrelsInfo()
{
    if (pBarrelsInfo_->getBarrelCount() > 0)
        pBarrelsInfo_->write(pIndexer_->getDirectory());
}

void IndexWriter::flushDocLen()
{
    if (pIndexBarrelWriter_) pIndexBarrelWriter_->flushDocLen();
}

void IndexWriter::createBarrelInfo()
{
    DVLOG(2)<< "=> IndexWriter::createBarrelInfo()...";

    pCurBarrelInfo_ = new BarrelInfo(pBarrelsInfo_->newBarrel(), 0, pIndexer_->pConfigurationManager_->indexStrategy_.indexLevel_, pIndexer_->getIndexCompressType());
    pCurBarrelInfo_->setSearchable(pIndexer_->isRealTime());
    pCurBarrelInfo_->setRealTime(pIndexer_->isRealTime());

    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pIndexBarrelWriter_->setBarrelInfo(pCurBarrelInfo_);
    pBarrelsInfo_->addBarrel(pCurBarrelInfo_);

    DVLOG(2)<< "<= IndexWriter::createBarrelInfo()";
}

void IndexWriter::checkbinlog()
{
    pIndexBarrelWriter_->checkbinlog();
}

void IndexWriter::deletebinlog()
{
    pIndexBarrelWriter_->deletebinlog();
}

void IndexWriter::indexDocument(IndexerDocument& doc)
{
    //boost::lock_guard<boost::mutex> lock(indexMutex_);

    if (!pCurBarrelInfo_) createBarrelInfo();

    if (pIndexer_->isRealTime())
    {
        ///If indexreader has not contained the in-memory barrel reader,
        ///The dirty flag should be set, so that the new query could open the in-memory barrel
        ///for real time query
        if (!pIndexer_->pIndexReader_->hasMemBarrelReader())
            pIndexer_->setDirty();
        if (pIndexBarrelWriter_->cacheFull())
        {
            LOG(INFO) << "IndexWriter::indexDocument() => realtime cache full...";
            pCurBarrelInfo_->setRealTime(false);
            flush();//
            deletebinlog();
            createBarrelInfo();
        }
    }

    DocId uniqueID;
    doc.getDocId(uniqueID);

    if (pCurBarrelInfo_->getBaseDocID() == BAD_DOCID ||
        pCurBarrelInfo_->getBaseDocID() > uniqueID.docId )
        pCurBarrelInfo_->addBaseDocID(uniqueID.colId,uniqueID.docId);

    pCurBarrelInfo_->updateMaxDoc(uniqueID.docId);
    pBarrelsInfo_->updateMaxDoc(uniqueID.docId);
    ++(pCurBarrelInfo_->nNumDocs);
    pIndexBarrelWriter_->addDocument(doc);
}

void IndexWriter::removeDocument(collectionid_t colID, docid_t docId)
{
    ///avoid of delete counting error
    Bitset* del_filter = pIndexer_->getIndexReader()->getDocFilter();
    if(del_filter && del_filter->test(docId))
        return;
    ///Perform deletion
    pIndexer_->getIndexReader()->delDocument(colID, docId);
    if (pIndexer_->pBTreeIndexer_)
        pIndexer_->pBTreeIndexer_->delDocument(pIndexer_->getBarrelsInfo()->maxDocId() + 1, docId);
    if (!pIndexBarrelWriter_->getDocFilter())
        pIndexBarrelWriter_->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
}

void IndexWriter::updateDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

//    if (doc.getId() > pBarrelsInfo_->maxDocId())
//        return;
    indexDocument(doc);
    removeDocument(uniqueID.colId,doc.getOldId());
}

void IndexWriter::updateRtypeDocument(IndexerDocument& oldDoc, IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

    // rtype update is difference from common update, because rtype update is an inplace update while
    // common update will remove old document and insert new document instead.
    // so we should handle this separately.
    // as result, the rtype property can not have any analyzer
    std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >&
        propertyValueList = doc.getPropertyList();
    std::map<IndexerPropertyConfig, IndexerDocumentPropertyType> oldDocMap;
    oldDoc.to_map(oldDocMap);
    for (std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >::const_iterator iter
            = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (iter->first.isIndex() && iter->first.isFilter())
        {
            map<IndexerPropertyConfig, IndexerDocumentPropertyType>::const_iterator it;
            if ( (it = oldDocMap.find(iter->first)) != oldDocMap.end() )
            {
                if (it->second == iter->second)
                    continue;
                if (it->first.isMultiValue())
                {
                    MultiValuePropertyType oldProp = boost::get<MultiValuePropertyType>(it->second);
                    for (MultiValuePropertyType::iterator multiIt = oldProp.begin(); multiIt != oldProp.end(); ++multiIt)
                        pIndexer_->getBTreeIndexer()->remove(iter->first.getName(), *multiIt, uniqueID.docId);
                }
                else
                {
                    PropertyType oldProp = boost::get<PropertyType>(it->second);
                    pIndexer_->getBTreeIndexer()->remove(iter->first.getName(), oldProp, uniqueID.docId);
                }
            }
            if (iter->first.isMultiValue())
            {
                MultiValuePropertyType prop = boost::get<MultiValuePropertyType>(iter->second);
                for (MultiValuePropertyType::iterator multiIt = prop.begin(); multiIt != prop.end(); ++multiIt)
                    pIndexer_->getBTreeIndexer()->add(iter->first.getName(), *multiIt, uniqueID.docId);
            }
            else
            {
                PropertyType prop = boost::get<PropertyType>(iter->second);
                pIndexer_->getBTreeIndexer()->add(iter->first.getName(), prop, uniqueID.docId);
            }
        }
    }
}

void IndexWriter::optimizeIndex()
{
    //boost::lock_guard<boost::mutex> lock(indexMutex_);
    flush();
    if(pIndexer_->isRealTime())
        deletebinlog();
    LOG(INFO)<<"Prepare optimize";
    pBarrelsInfo_->printBarrelsInfo(pIndexer_->pDirectory_);
    pIndexMergeManager_->optimizeIndex();
    while (pBarrelsInfo_->getBarrelCount() > 1)
        sleep(0);
    LOG(INFO)<<"Finish optimize";
    pBarrelsInfo_->printBarrelsInfo(pIndexer_->pDirectory_);
}

//void IndexWriter::lazyOptimizeIndex(int calltype)
//{
//    using namespace boost::posix_time;
//    using namespace boost::gregorian;
//    using namespace izenelib::util;
//
//    boost::posix_time::ptime now = second_clock::local_time();
//    boost::gregorian::date date = now.date();
//    boost::posix_time::time_duration du = now.time_of_day();
//    int dow = date.day_of_week();
//    int month = date.month();
//    int day = date.day();
//    int hour = du.hours();
//    int minute = du.minutes();
//    if (scheduleExpression_.matches(minute, hour, day, month, dow))
//    {
//        boost::lock_guard<boost::mutex> lock(indexMutex_);
//        flush();
//        if(pIndexer_->isRealTime())
//            deletebinlog();
//        pIndexMergeManager_->optimizeIndex();
//    }
//}
//
//void IndexWriter::scheduleOptimizeTask(std::string expression, string uuid)
//{
//    scheduleExpression_.setExpression(expression);
//
//    const string optimizeJob = "optimizeindex"+uuid;
//    ///we need an uuid here because scheduler is an singleton in the system
//    Scheduler::removeJob(optimizeJob);
//
//    optimizeJobDesc_ = optimizeJob;
//    boost::function<void (int)> task = boost::bind(&IndexWriter::lazyOptimizeIndex,this, _1);
//    Scheduler::addJob(optimizeJob, 60*1000, 0, task);
//}
//
void IndexWriter::setIndexMode(bool realtime)
{
    if (!pIndexBarrelWriter_)
    {
        pIndexBarrelWriter_ = new IndexBarrelWriter(pIndexer_);
        pIndexBarrelWriter_->setCollectionsMeta(pIndexer_->getCollectionsMeta());
    }
    pIndexBarrelWriter_->setIndexMode(realtime);
}

}

NS_IZENELIB_IR_END
