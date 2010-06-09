/**
* @file        OfflineIndexMergerMergerr.h
* @version     SF1 v5.0
* @brief OPT index merge algorithm
*/
#ifndef OPTIMIZEINDEXMERGER_H
#define OPTIMIZEINDEXMERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* This class has implemented the optimize index merge algorithm that will merge all the barrels into a single index barrel
*/
class OfflineIndexMerger : public IndexMerger
{
public:
    OfflineIndexMerger(Indexer* pIndexer, unsigned int numBarrels);
    virtual ~OfflineIndexMerger(void);
public:
    void setBarrels(unsigned int numBarrels);

    void addBarrel(MergeBarrelEntry* pEntry);

    void endMerge();
protected:
    MergeBarrel* pMergeBarrel_;
};

}

NS_IZENELIB_IR_END

#endif

