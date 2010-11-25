/**
* @file        IndexMergePolicy.h
* @version     SF1 v5.0
* @brief interface of index merge policy
*/
#ifndef INDEX_MERGE_POLICY_H
#define INDEX_MERGE_POLICY_H

#include <types.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class IndexMerger;
class MergeBarrelEntry;

/**
 * It defines the interface of index merge policy.
 */
class IndexMergePolicy
{
public:
    IndexMergePolicy() :pIndexMerger_(NULL) {}

    virtual ~IndexMergePolicy() {};

    void setIndexMerger(IndexMerger* pIndexMerger) { pIndexMerger_ = pIndexMerger; }

    /**
     * add new index barrel to merge.
     * @param pEntry the index barrel
     * @note derived class is responsible to delete @p pEntry
     */
    virtual void addBarrel(MergeBarrelEntry* pEntry) = 0;

    /**
     * denote the merge is over, used to clear some resources for merging.
     */
    virtual void endMerge() = 0;

protected:
    IndexMerger* pIndexMerger_;
};

}

NS_IZENELIB_IR_END

#endif

