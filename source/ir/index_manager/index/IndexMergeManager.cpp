#include <boost/thread.hpp>
#include <cassert>

#include <ir/index_manager/index/IndexMergeManager.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/GPartitionMerger.h>
#include <ir/index_manager/index/MultiWayMerger.h>
#include <ir/index_manager/index/GPartitionMerger.h>
#include <ir/index_manager/index/DefaultMerger.h>
#include <ir/index_manager/index/DBTMerger.h>
#include <ir/index_manager/index/OptimizeMerger.h>
#include <ir/index_manager/index/IndexReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexMergeManager::IndexMergeManager(Indexer* pIndexer, bool isAsyncMerge)
    :pIndexer_(pIndexer)
    ,pBarrelsInfo_(NULL)
    ,pAddMerger_(NULL)
    ,pOptimizeMerger_(NULL)
    ,pMergeThread_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"no"))
        pAddMerger_ = NULL;
    else if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"dbt"))
        pAddMerger_ = new DBTMerger(pIndexer_);
    else if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"mway"))
        pAddMerger_ = new MultiWayMerger(pIndexer_);	
    else if(!strcasecmp(pIndexer_->pConfigurationManager_->mergeStrategy_.param_.c_str(),"gpart"))
        pAddMerger_ = new GPartitionMerger(pIndexer_);
    else
        pAddMerger_ = new DefaultMerger(pIndexer_);

    pOptimizeMerger_ = new OptimizeMerger(pIndexer_, pBarrelsInfo_->getBarrelCount());

    if(isAsyncMerge)
        run();
}

IndexMergeManager::~IndexMergeManager()
{
    stop();

    delete pMergeThread_;
    delete pAddMerger_;
    delete pOptimizeMerger_;
}

void IndexMergeManager::run()
{
    assert(! pMergeThread_ && "the merge thread should not be running before");
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

void IndexMergeManager::addToMerge(BarrelInfo* pBarrelInfo)
{
    if(pMergeThread_)
    {
        MergeOP op(ADD);
        op.pBarrelInfo = pBarrelInfo;
        tasks_.push(op);
    }
    else
    {
        pAddMerger_->addToMerge(pBarrelsInfo_, pBarrelInfo);
    }
}

void IndexMergeManager::optimizeIndex()
{
    if(pMergeThread_)
    {
        tasks_.clear();
        MergeOP op(OPTIMIZE);
        tasks_.push(op);
    }
    else
    {
        optimizeIndexImpl();
    }
}

void IndexMergeManager::optimizeIndexImpl()
{
    pOptimizeMerger_->setBarrels(pBarrelsInfo_->getBarrelCount());
    if(pIndexer_->getIndexReader()->getDocFilter())
        pOptimizeMerger_->setDocFilter(pIndexer_->getIndexReader()->getDocFilter());
    pOptimizeMerger_->merge(pBarrelsInfo_);
    pIndexer_->getIndexReader()->delDocFilter();
    pOptimizeMerger_->setDocFilter(NULL);
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
        case ADD:
            {
            assert(op.pBarrelInfo);

            pAddMerger_->addToMerge(pBarrelsInfo_, op.pBarrelInfo);
            }
            break;
        case OPTIMIZE:
	    {
            // clear the status of add merger
            pAddMerger_->endMerge();

            optimizeIndexImpl();
            }
            break;
        case NOOP:
            return;
        }
    }
}

}

NS_IZENELIB_IR_END

