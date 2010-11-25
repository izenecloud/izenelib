/**
* @file        OptimizeMerger.h
* @version     SF1 v5.0
* @brief OPT index merge algorithm
*/
#ifndef OPTIMIZEINDEXMERGER_H
#define OPTIMIZEINDEXMERGER_H

#include <ir/index_manager/index/IndexMergePolicy.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class MergeBarrel;

/**
* This class has implemented the optimize index merge algorithm that will merge all the barrels into a single index barrel
*/
class OptimizeMerger : public IndexMergePolicy
{
public:
    OptimizeMerger(unsigned int numBarrels);
    virtual ~OptimizeMerger(void);
public:
    virtual void addBarrel(MergeBarrelEntry* pEntry);

    virtual void endMerge();
protected:
    MergeBarrel* pMergeBarrel_;
};

}

NS_IZENELIB_IR_END

#endif

