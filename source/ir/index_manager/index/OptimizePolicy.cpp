#include <ir/index_manager/index/OptimizePolicy.h>
#include <ir/index_manager/index/IndexMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizePolicy::OptimizePolicy(unsigned int numBarrels)
        :pBarrelQueue_(new MergeBarrelQueue("_mid_0_",numBarrels+5))
{
}

OptimizePolicy::~OptimizePolicy(void)
{
    delete pBarrelQueue_;
}

void OptimizePolicy::addBarrel(MergeBarrelEntry* pEntry)
{
    pBarrelQueue_->put(pEntry);
}
void OptimizePolicy::endMerge()
{
    pIndexMerger_->mergeBarrel(pBarrelQueue_);
}

