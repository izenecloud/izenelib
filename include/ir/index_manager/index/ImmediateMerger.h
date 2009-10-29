#ifndef IMMEDIATE_MERGER_H
#define IMMEDIATE_MERGER_H

#include <ir/index_manager/index/IndexMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* This class has implemented the optimize index merge algorithm that will merge all the barrels into a single index barrel
*/
class ImmediateMerger : public IndexMerger
{
public:
    ImmediateMerger(Directory* pSrcDirectory);
    virtual ~ImmediateMerger(void);
public:
    void addBarrel(MergeBarrelEntry* pEntry);

    void endMerge();
protected:
    MergeBarrel* pMergeBarrel_;
    int mergeTimes_;
};

}

NS_IZENELIB_IR_END

#endif
