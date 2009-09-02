#include <ir/index_manager/index/OptimizeMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizeMerger::OptimizeMerger(Directory* pSrDirectory)
        :IndexMerger(pSrDirectory)
{
    string s = "_mid_0_";
    pMergeBarrel_ = new MergeBarrel(s.c_str(),100);
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

