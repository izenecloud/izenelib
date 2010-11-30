/**
* @file        MockIndexMerger.h
* @author      Jun
* @version     SF1 v5.0
* @brief IndexMerger mocker.
*/
#ifndef MOCK_INDEX_MERGER_H
#define MOCK_INDEX_MERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/IndexMergePolicy.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

/**
 * A mocker to simulate IndexMerger.
 * It triggers "merge" as usual, and write file "barrels",
 * while not loading or writing true barrel files, such as ".voc", ".fdi", etc.
 */
class MockIndexMerger : public IndexMerger
{
public:
    MockIndexMerger(Indexer* pIndexer, IndexMergePolicy* pMergePolicy)
        :IndexMerger(pIndexer, pMergePolicy)
    {}

protected:
    /**
     * merge a merge barrel which contains more than one index barrels
     * @param pBarrelQueue merge barrel queue
     * @note in this overriding function, the calls of barrel files loading
     * and writing are committed out deliberately for mock purpose.
     */
    virtual void mergeBarrel(MergeBarrelQueue* pBarrelQueue) {
        DVLOG(2)<< "=> MockIndexMerger::mergeBarrel()";

        triggerMerge_ = true;
        //pBarrelQueue->load();
        std::string newBarrelName = pBarrelQueue->getIdentifier();

        //outputNewBarrel(pBarrelQueue, newBarrelName);

        BarrelInfo* pNewBarrelInfo = createNewBarrelInfo(pBarrelQueue, newBarrelName);

        MergeBarrelEntry* pNewEntry = new MergeBarrelEntry(pDirectory_, pNewBarrelInfo);
        pMergePolicy_->addBarrel(pNewEntry);

        DVLOG(2)<< "<= MockIndexMerger::mergeBarrel()";
    }
};

}
NS_IZENELIB_IR_END
#endif
