#include <boost/thread/locks.hpp>
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
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    IndexManagerConfig* pConfig = pIndexer_->getIndexManagerConfig();
    const char* mergeStrategyStr = pConfig->mergeStrategy_.param_.c_str();

    if(strcasecmp(mergeStrategyStr,"no"))
        pAddMerger_ = new IndexMerger(pIndexer_, new BTPolicy);

    if(pConfig->mergeStrategy_.isAsync_)
        run();
}

IndexMergeManager::~IndexMergeManager()
{
    // notify the merge thread in case of it was paused
    pauseMergeCond_.notify_all();

    stop();

    delete pMergeThread_;
    delete pAddMerger_;
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
    DVLOG(2) << "=> IndexMergeManager::waitForMergeFinish()";
    if(pMergeThread_)
    {
        joinMergeThread();
        run();
    }
    DVLOG(2) << "<= IndexMergeManager::waitForMergeFinish()";
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
    IndexMerger optimizeMerger(pIndexer_, new OptimizePolicy(pBarrelsInfo_->getBarrelCount()));

    IndexReader* pIndexReader = pIndexer_->getIndexReader();
    if(BitVector* pBitVector = pIndexReader->getDocFilter())
        optimizeMerger.setDocFilter(pBitVector);
    optimizeMerger.merge(pBarrelsInfo_);

    pIndexReader->delDocFilter();
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
    if(pMergeThread_)
    {
        boost::lock_guard<boost::mutex> lock(pauseMergeMutex_);
        isPauseMerge_ = true;
    }
    DVLOG(2) << "<= IndexMergeManager::pauseMerge()";
}

void IndexMergeManager::resumeMerge()
{
    DVLOG(2) << "=> IndexMergeManager::resumeMerge()";
    if(pMergeThread_)
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

