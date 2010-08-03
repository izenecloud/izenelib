#include <boost/thread.hpp>

#include <ir/index_manager/index/IndexMergeManager.h>
#include <ir/index_manager/index/GPartitionMerger.h>
#include <ir/index_manager/index/DefaultMerger.h>
#include <ir/index_manager/index/DBTMerger.h>
#include <ir/index_manager/index/OfflineIndexMerger.h>
#include <ir/index_manager/index/IndexReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexMergeManager::IndexMergeManager(Indexer* pIndexer)
    :pIndexer_(pIndexer)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"dbt"))
        indexMergers_[ONLINE] = new DBTMerger(pIndexer_);
    else
        indexMergers_[ONLINE] = new DefaultMerger(pIndexer_);

    indexMergers_[OFFLINE] = new OfflineIndexMerger(pIndexer_, pBarrelsInfo_->getBarrelCount());
}

IndexMergeManager::~IndexMergeManager()
{
}

void IndexMergeManager::run()
{
    mergethread_.reset(new boost::thread(boost::bind(&IndexMergeManager::mergeIndex, this)));
    mergethread_->detach();
}

void IndexMergeManager::stop()
{
    tasks_.clear();
    MergeOP op;
    op.opType = NOOP;
    tasks_.push(op);
    mergethread_->join();
    for(std::map<MergeOPType, IndexMerger*>::iterator iter = indexMergers_.begin();
              iter != indexMergers_.end(); ++iter)
        delete iter->second;
}

void IndexMergeManager::triggerMerge(BarrelInfo* pBarrelInfo)
{
    MergeOP op;
    op.opType = ONLINE;
    op.pBarrelInfo = pBarrelInfo;
    tasks_.push(op);
}

void IndexMergeManager::mergeIndex()
{
    while(true)
    {
        MergeOP op;
        tasks_.pop(op);
        switch(op.opType)
        {
        case ONLINE:
            {
            IndexMerger* pIndexMerger = indexMergers_[op.opType];
            pIndexMerger->addToMerge(pBarrelsInfo_, op.pBarrelInfo);
            }
            break;
        case FINISH:
            {
            IndexMerger* pIndexMerger = indexMergers_[ONLINE];
            pIndexMerger->endMerge();
            BarrelInfo* pBaInfo;
            pBarrelsInfo_->startIterator();
            while (pBarrelsInfo_->hasNext())
            {
                pBaInfo = pBarrelsInfo_->next();
                pBaInfo->setSearchable(true);
            }
            pBarrelsInfo_->setLock(true);
            pIndexMerger->updateBarrels(pBarrelsInfo_);

            pBarrelsInfo_->write(pIndexer_->getDirectory());
            pBarrelsInfo_->setLock(false);
            }
            break;
        case OFFLINE:
	    {
            OfflineIndexMerger* pIndexMerger = reinterpret_cast<OfflineIndexMerger*>(indexMergers_[op.opType]);
            pIndexMerger->setBarrels(pBarrelsInfo_->getBarrelCount());
            if(pIndexer_->getIndexReader()->getDocFilter())
                pIndexMerger->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
            pIndexMerger->merge(pBarrelsInfo_);
            pIndexer_->getIndexReader()->delDocFilter();
            pIndexMerger->setDocFilter(NULL);
            BarrelInfo* pBaInfo;
            pBarrelsInfo_->startIterator();
            while (pBarrelsInfo_->hasNext())
            {
                pBaInfo = pBarrelsInfo_->next();
                pBaInfo->setSearchable(true);
            }
            pBarrelsInfo_->setLock(true);
            pIndexMerger->updateBarrels(pBarrelsInfo_);

            pBarrelsInfo_->write(pIndexer_->getDirectory());
            pBarrelsInfo_->setLock(false);
            }
            break;
        case NOOP:
            return;
        }
    }
}

void IndexMergeManager::optimizeIndex()
{
    MergeOP op;
    if(tasks_.empty())
    {
        op.opType = OFFLINE;
    }
    else
    {
        op.opType = FINISH;
    }
    tasks_.push(op);
}

}

NS_IZENELIB_IR_END

