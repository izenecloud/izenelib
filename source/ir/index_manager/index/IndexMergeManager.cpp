#include <boost/thread.hpp>
#include <cassert>

#include <ir/index_manager/index/IndexMergeManager.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/BTMerger.h>
#include <ir/index_manager/index/OptimizeMerger.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexMergeManager::IndexMergeManager(Indexer* pIndexer)
    :pIndexer_(pIndexer)
    ,pBarrelsInfo_(NULL)
    ,pAddMerger_(NULL)
    ,pOptimizeMerger_(NULL)
    ,pMergeThread_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    IndexManagerConfig* pConfig = pIndexer_->getIndexManagerConfig();
    const char* mergeStrategyStr = pConfig->mergeStrategy_.param_.c_str();

    if(strcasecmp(mergeStrategyStr,"no"))
        pAddMerger_ = new BTMerger(pIndexer_);

    pOptimizeMerger_ = new OptimizeMerger(pIndexer_, pBarrelsInfo_->getBarrelCount());

    if(pConfig->mergeStrategy_.isAsync_)
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
        MergeOP op(EXIT_MERGE);
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
        MergeOP op(ADD_BARREL);
        op.pBarrelInfo = pBarrelInfo;
        tasks_.push(op);
    }
    else
    {
        if(pAddMerger_)
            pAddMerger_->addToMerge(pBarrelsInfo_, pBarrelInfo);
    }
}

void IndexMergeManager::optimizeIndex()
{
    if(pMergeThread_)
    {
        tasks_.clear();
        MergeOP op(OPTIMIZE_ALL);
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
        case ADD_BARREL:
            {
            assert(op.pBarrelInfo);

            if(pAddMerger_)
                pAddMerger_->addToMerge(pBarrelsInfo_, op.pBarrelInfo);
            }
            break;
        case OPTIMIZE_ALL:
	    {
            // clear the status of add merger
            if(pAddMerger_)
                pAddMerger_->endMerge();

            optimizeIndexImpl();
            }
            break;
        case EXIT_MERGE:
            return;
        }
    }
}

}

NS_IZENELIB_IR_END

