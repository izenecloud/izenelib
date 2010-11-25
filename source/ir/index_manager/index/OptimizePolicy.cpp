#include <ir/index_manager/index/OptimizePolicy.h>
#include <ir/index_manager/index/IndexMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizePolicy::OptimizePolicy(unsigned int numBarrels)
        :pMergeBarrel_(new MergeBarrel("_mid_0_",numBarrels+5))
{
}

OptimizePolicy::~OptimizePolicy(void)
{
    delete pMergeBarrel_;
}

void OptimizePolicy::addBarrel(MergeBarrelEntry* pEntry)
{
    pMergeBarrel_->put(pEntry);
}
void OptimizePolicy::endMerge()
{
    pIndexMerger_->mergeBarrel(pMergeBarrel_);
}

