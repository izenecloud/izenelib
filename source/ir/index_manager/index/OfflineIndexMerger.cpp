#include <ir/index_manager/index/OfflineIndexMerger.h>

using namespace izenelib::ir::indexmanager;

OfflineIndexMerger::OfflineIndexMerger(Indexer* pIndexer, unsigned int numBarrels)
        :IndexMerger(pIndexer)
        ,pMergeBarrel_(NULL)
{
    setBarrels(numBarrels);
}

OfflineIndexMerger::~OfflineIndexMerger(void)
{
    if(pMergeBarrel_)
        delete pMergeBarrel_;
}

void OfflineIndexMerger::setBarrels(unsigned int numBarrels)
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

void OfflineIndexMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    pMergeBarrel_->put(pEntry);
}
void OfflineIndexMerger::endMerge()
{
    mergeBarrel(pMergeBarrel_);
}

