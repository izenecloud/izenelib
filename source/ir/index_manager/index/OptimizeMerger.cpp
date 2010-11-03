#include <ir/index_manager/index/OptimizeMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizeMerger::OptimizeMerger(Indexer* pIndexer, unsigned int numBarrels)
        :IndexMerger(pIndexer)
        ,pMergeBarrel_(NULL)
{
    setBarrels(numBarrels);
}

OptimizeMerger::~OptimizeMerger(void)
{
    if(pMergeBarrel_)
        delete pMergeBarrel_;
}

void OptimizeMerger::setBarrels(unsigned int numBarrels)
{
    if(numBarrels > 0)
    {
        if(pMergeBarrel_)
        {
            delete pMergeBarrel_;
        }
        string s = "_mid_0_";
        pMergeBarrel_ = new MergeBarrel(s.c_str(),numBarrels+5);
    }
}

void OptimizeMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    pMergeBarrel_->put(pEntry);
}
void OptimizeMerger::endMerge()
{
    mergeBarrel(pMergeBarrel_);
}

