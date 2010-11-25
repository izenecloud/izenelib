#include <ir/index_manager/index/OptimizeMerger.h>
#include <ir/index_manager/index/IndexMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizeMerger::OptimizeMerger(unsigned int numBarrels)
        :pMergeBarrel_(new MergeBarrel("_mid_0_",numBarrels+5))
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
    pIndexMerger_->mergeBarrel(pMergeBarrel_);
}

