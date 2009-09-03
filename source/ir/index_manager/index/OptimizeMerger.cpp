#include <ir/index_manager/index/OptimizeMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizeMerger::OptimizeMerger(Directory* pSrDirectory, unsigned int numBarrels)
        :IndexMerger(pSrDirectory)
{
    string s = "_mid_0_";
    pMergeBarrel_ = new MergeBarrel(s.c_str(),numBarrels+5);
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

