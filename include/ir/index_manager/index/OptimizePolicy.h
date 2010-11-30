/**
* @file        OptimizePolicy.h
* @version     SF1 v5.0
* @brief OPT index merge algorithm
*/
#ifndef OPTIMIZE_POLICY_H
#define OPTIMIZE_POLICY_H

#include <ir/index_manager/index/IndexMergePolicy.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class MergeBarrelQueue;

/**
* This class has implemented the optimize index merge algorithm that will merge all the barrels into a single index barrel
*/
class OptimizePolicy : public IndexMergePolicy
{
public:
    OptimizePolicy(unsigned int numBarrels);
    virtual ~OptimizePolicy(void);
public:
    virtual void addBarrel(MergeBarrelEntry* pEntry);

    virtual void endMerge();
protected:
    MergeBarrelQueue* pBarrelQueue_;
};

}

NS_IZENELIB_IR_END

#endif

