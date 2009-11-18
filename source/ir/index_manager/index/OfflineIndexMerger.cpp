#include <ir/index_manager/index/OfflineIndexMerger.h>

using namespace izenelib::ir::indexmanager;

OfflineIndexMerger::OfflineIndexMerger(Indexer* pIndexer, unsigned int numBarrels)
        :IndexMerger(pIndexer)
{
    string s = "_mid_0_";
    pMergeBarrel_ = new MergeBarrel(s.c_str(),numBarrels+5);
}

OfflineIndexMerger::~OfflineIndexMerger(void)
{
    delete pMergeBarrel_;
}
void OfflineIndexMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    pMergeBarrel_->put(pEntry);
}
void OfflineIndexMerger::endMerge()
{
    mergeBarrel(pMergeBarrel_);
}

