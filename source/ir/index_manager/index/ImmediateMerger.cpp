#include <ir/index_manager/index/ImmediateMerger.h>

using namespace izenelib::ir::indexmanager;

ImmediateMerger::ImmediateMerger(Directory* pSrDirectory)
        :IndexMerger(pSrDirectory)
        ,mergeTimes_(0)
{
    string s = "_mid_0";
    pMergeBarrel_ = new MergeBarrel(s.c_str(),2);
}

ImmediateMerger::~ImmediateMerger(void)
{
    delete pMergeBarrel_;
}
void ImmediateMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    pMergeBarrel_->put(pEntry);
    if(pMergeBarrel_->size() == 2)
    {
        mergeBarrel(pMergeBarrel_);
        mergeTimes_++;
        string str("_mid_");
        str += mergeTimes_;
        pMergeBarrel_->setIdentifier(str);
    }
}
void ImmediateMerger::endMerge()
{
}


