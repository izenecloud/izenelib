#include <boost/thread.hpp>

#include <ir/index_manager/index/IndexMergeManager.h>
#include <ir/index_manager/index/GPartitionMerger.h>
#include <ir/index_manager/index/MultiWayMerger.h>
#include <ir/index_manager/index/OfflineIndexMerger.h>
#include <ir/index_manager/index/IndexReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexMergeManager::IndexMergeManager(Indexer* pIndexer)
    :pIndexer_(pIndexer)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    indexMergers_[INDEX] = new MultiWayMerger(pIndexer_);
    indexMergers_[UPDATE] = new MultiWayMerger(pIndexer_);
    indexMergers_[OFFLINE] = new OfflineIndexMerger(pIndexer_, pBarrelsInfo_->getBarrelCount());
}

IndexMergeManager::~IndexMergeManager()
{
    stop();
    for(std::map<MergeOPType, IndexMerger*>::iterator iter = indexMergers_.begin();
              iter != indexMergers_.end(); ++iter)
        delete iter->second;
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
}

void IndexMergeManager::triggerMerge(BarrelInfo* pBarrelInfo, bool update)
{
    MergeOP op;
    op.opType = update? UPDATE : INDEX;
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
        case INDEX:
        case UPDATE:
            {
            IndexMerger* pIndexMerger = indexMergers_[op.opType];
            pIndexMerger->addToMerge(pBarrelsInfo_, op.pBarrelInfo);
            }
            break;
        case OFFLINE:
	    {
            OfflineIndexMerger* pIndexMerger = reinterpret_cast<OfflineIndexMerger*>(indexMergers_[op.opType]);
            pIndexMerger->setBarrels(pBarrelsInfo_->getBarrelCount());
            pIndexMerger->merge(pBarrelsInfo_, INDEX_ONLY);
            pIndexMerger->setBarrels(pBarrelsInfo_->getBarrelCount());
            pIndexMerger->merge(pBarrelsInfo_, UPDATE_ONLY);
            pIndexMerger->setBarrels(pBarrelsInfo_->getBarrelCount());
            if(pIndexer_->getIndexReader()->getDocFilter())
                pIndexMerger->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
            pIndexMerger->merge(pBarrelsInfo_);
            pIndexer_->getIndexReader()->delDocFilter();
            pIndexMerger->setDocFilter(NULL);
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
    op.opType = OFFLINE;
    tasks_.push(op);
}

}

NS_IZENELIB_IR_END

