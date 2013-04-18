#include <boost/thread.hpp>
#include <cassert>

#include <ir/index_manager/index/IndexMergeManager.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/BTPolicy.h>
#include <ir/index_manager/index/OptimizePolicy.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

IndexMergeManager::IndexMergeManager(Indexer* pIndexer)
    :pIndexer_(pIndexer)
    ,pBarrelsInfo_(NULL)
    ,pAddMerger_(NULL)
    ,pMergeThread_(NULL)
    ,isPauseMerge_(false)
    ,isAsync_(pIndexer->getIndexManagerConfig()->mergeStrategy_.isAsync_)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    IndexManagerConfig* pConfig = pIndexer_->getIndexManagerConfig();
    const char* mergeStrategyStr = pConfig->mergeStrategy_.param_.c_str();

    if(strcasecmp(mergeStrategyStr,"no"))
        pAddMerger_ = new IndexMerger(pIndexer_, new BTPolicy);

    if(isAsync_)
        run();
}

IndexMergeManager::~IndexMergeManager()
{
    // notify the merge thread in case of it was paused
    pauseMergeCond_.notify_all();

    if(isAsync_)
    {
        boost::lock_guard<boost::mutex> lock(mergeThreadMutex_);
        tasks_.clear();
        joinMergeThread();
    }

    if(pAddMerger_)
        delete pAddMerger_;
}

void IndexMergeManager::run()
{
    assert(isAsync_ && "to create the merge thread, IndexManagerConfig._mergestrategy.isAsync_ should be configured to true");
    assert(! pMergeThread_ && "the merge thread should not be running before");
    pMergeThread_ = new boost::thread(boost::bind(&IndexMergeManager::mergeIndex, this));
}

void IndexMergeManager::joinMergeThread()
{
    assert(isAsync_);

    MergeOP op(EXIT_MERGE);
    tasks_.push(op);

    DVLOG(2) << "IndexMergeManager::joinMergeThreadImpl() => pMergeThread_->join()...";
    pMergeThread_->join();
    DVLOG(2) << "IndexMergeManager::joinMergeThreadImpl() <= pMergeThread_->join()";

    delete pMergeThread_;
    pMergeThread_ = NULL;
}

void IndexMergeManager::waitForMergeFinish()
{
    DVLOG(2) << "=> IndexMergeManager::waitForMergeFinish()";
    if(isAsync_)
    {
        boost::lock_guard<boost::mutex> lock(mergeThreadMutex_);
        joinMergeThread();
        run();
    }
    DVLOG(2) << "<= IndexMergeManager::waitForMergeFinish()";
}

void IndexMergeManager::addToMerge(BarrelInfo* pBarrelInfo)
{
    if(isAsync_)
    {
        MergeOP op(ADD_BARREL);
        op.pBarrelInfo = pBarrelInfo;
        tasks_.push(op);
    }
    else
    {
        if(pAddMerger_)
            pAddMerger_->addToMerge(pBarrelInfo);
    }
}

void IndexMergeManager::optimizeIndex()
{
    if(isAsync_)
    {
        boost::lock_guard<boost::mutex> lock(mergeThreadMutex_);
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
    IndexMerger optimizeMerger(pIndexer_,
                               new OptimizePolicy(pBarrelsInfo_->getBarrelCount()));

    optimizeMerger.mergeBarrels();
    pIndexer_->getIndexReader();
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
                pAddMerger_->addToMerge(op.pBarrelInfo);
            }
            break;
        case OPTIMIZE_ALL:
	    {
            // to clear the status of add merger, renew it
            if(pAddMerger_)
            {
                delete pAddMerger_;
                pAddMerger_ = new IndexMerger(pIndexer_, new BTPolicy);
            }

            optimizeIndexImpl();
            }
            break;
        case EXIT_MERGE:
            return;
        }
    }
}

void IndexMergeManager::pauseMerge()
{
    DVLOG(2) << "=> IndexMergeManager::pauseMerge()";
    if(isAsync_)
    {
        boost::lock_guard<boost::mutex> lock(pauseMergeMutex_);
        isPauseMerge_ = true;
    }
    DVLOG(2) << "<= IndexMergeManager::pauseMerge()";
}

void IndexMergeManager::resumeMerge()
{
    DVLOG(2) << "=> IndexMergeManager::resumeMerge()";
    if(isAsync_)
    {
        {
            boost::lock_guard<boost::mutex> lock(pauseMergeMutex_);
            isPauseMerge_ = false;
        }
        pauseMergeCond_.notify_all();
    }
    DVLOG(2) << "<= IndexMergeManager::resumeMerge()";
}

}

NS_IZENELIB_IR_END

