#include <ir/index_manager/index/OptimizeMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizeMerger::OptimizeMerger(Indexer* pIndexer, unsigned int numBarrels)
        :IndexMerger(pIndexer)
        ,pMergeBarrel_(new MergeBarrel("_mid_0_",numBarrels+5))
{
}

OptimizeMerger::~OptimizeMerger(void)
{
    delete pMergeBarrel_;
}

void OptimizeMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    pMergeBarrel_->put(pEntry);
}
void OptimizeMerger::endMerge()
{
    mergeBarrel(pMergeBarrel_);
}

