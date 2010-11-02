#include <boost/thread.hpp>
#include <cassert>

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
    ,pBarrelsInfo_(NULL)
    ,pMergeThread_(NULL)
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
    stop();
    delete pMergeThread_;

    for(std::map<MergeOPType, IndexMerger*>::iterator iter = indexMergers_.begin();
              iter != indexMergers_.end(); ++iter)
        delete iter->second;
}

void IndexMergeManager::run()
{
    if(! pMergeThread_)
        pMergeThread_ = new boost::thread(boost::bind(&IndexMergeManager::mergeIndex, this));
}

void IndexMergeManager::stop()
{
    tasks_.clear();
    joinMergeThread();
}

void IndexMergeManager::joinMergeThread()
{
    if(pMergeThread_)
    {
        MergeOP op(NOOP);
        tasks_.push(op);

        DVLOG(2) << "IndexMergeManager::joinMergeThread() => pMergeThread_->join()...";
        pMergeThread_->join();
        DVLOG(2) << "IndexMergeManager::joinMergeThread() <= pMergeThread_->join()";

        delete pMergeThread_;
        pMergeThread_ = NULL;
    }
}

void IndexMergeManager::waitForMergeFinish()
{
    if(pMergeThread_)
    {
        joinMergeThread();
        run();
    }
}

void IndexMergeManager::triggerMerge(BarrelInfo* pBarrelInfo)
{
    MergeOP op(ONLINE);
    op.pBarrelInfo = pBarrelInfo;
    tasks_.push(op);
}

void IndexMergeManager::mergeIndex()
{
    while(true)
    {
        MergeOP op;
        tasks_.pop(op);
        DVLOG(2) << "IndexMergeManager::mergeIndex(), opType: " << op.opType;
        switch(op.opType)
        {
        case ONLINE:
            {
            assert(op.pBarrelInfo);

            IndexMerger* pIndexMerger = indexMergers_[op.opType];
            pIndexMerger->addToMerge(pBarrelsInfo_, op.pBarrelInfo);
            }
            break;
        case OFFLINE:
	    {
            // clear the status of online merger
            IndexMerger* pOnlineMerger = indexMergers_[ONLINE];
            pOnlineMerger->endMerge();

            // merge all barrels into one
            OfflineIndexMerger* pOfflineMerger = dynamic_cast<OfflineIndexMerger*>(indexMergers_[op.opType]);
            assert(pOfflineMerger);

            pOfflineMerger->setBarrels(pBarrelsInfo_->getBarrelCount());
            if(pIndexer_->getIndexReader()->getDocFilter())
                pOfflineMerger->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
            pOfflineMerger->merge(pBarrelsInfo_);
            pIndexer_->getIndexReader()->delDocFilter();
            pOfflineMerger->setDocFilter(NULL);
            }
            break;
        case NOOP:
            return;
        }
    }
}

void IndexMergeManager::optimizeIndex()
{
    tasks_.clear();

    MergeOP op(OFFLINE);
    tasks_.push(op);
}

}

NS_IZENELIB_IR_END

