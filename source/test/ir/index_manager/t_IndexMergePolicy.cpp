#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "IndexerTestFixture.h"
#include <ir/index_manager/index/MockIndexMerger.h>
#include <ir/index_manager/index/BTMerger.h>
#include <ir/index_manager/index/OptimizeMerger.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <string>
#include <sstream> // std::ostringstream

using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;

/**
 * The parameters in testing IndexMergePolicy module.
 */
struct BarrelConfig
{
    enum {
        MAX_BARREL_NUM = 100 ///< maximum barrels number allowed
    };

    /**
     * doc number in each barrel.
     * if docNums_[k] is the 1st element of value 0,
     * then k barrels each with doc number of docNums_[i] would be created,
     * the values of element starting from k are ignored.
     */
    int docNums_[MAX_BARREL_NUM]; 

    /**
     * the number of barrels after merge.
     * this value would be checked when merge is finished.
     */
    int mergedBarrelNum_; 

    /**
     * Get string which outputs each parameters.
     * @return output string
     */
    std::string str() const {
        std::ostringstream oss;
        oss << "BarrelConfig, docNums_: (";
        BOOST_FOREACH(int docNum, docNums_)
        {
            if(docNum == 0)
                break;
            oss << docNum << ", ";
        }
        oss << "), mergedBarrelNum_: " << mergedBarrelNum_ << ")";

        return oss.str();
    }
};

/**
 * create a new @c BarrelInfo.
 * @param pBarrelsInfo the @c BarrelsInfo instance
 * @param docNum the number of doc contained in new @c BarrelInfo
 * @return the new created instance
 */
BarrelInfo* newBarrelInfo(BarrelsInfo* pBarrelsInfo, int docNum)
{
    BarrelInfo* pNewBarrelInfo = new BarrelInfo(pBarrelsInfo->newBarrel(), docNum);
    pNewBarrelInfo->baseDocIDMap[IndexerTestFixture::COLLECTION_ID] = pBarrelsInfo->maxDocId() + 1;
    pNewBarrelInfo->maxDocId = pBarrelsInfo->maxDocId() + docNum;
    pNewBarrelInfo->setSearchable(true);
    pNewBarrelInfo->setWriter(NULL);
    pBarrelsInfo->addBarrel(pNewBarrelInfo,false);
    pBarrelsInfo->updateMaxDoc(pNewBarrelInfo->maxDocId);

    return pNewBarrelInfo;
}

/**
 * check function @c IndexMerger::addToMerge using @p pIndexMergePolicy.
 * @p pIndexMergePolicy the instance of merge policy
 * @p barrelConfig the parameter of barrels to merge
 */
void checkAddToMerge(IndexMergePolicy* pIndexMergePolicy, const BarrelConfig& barrelConfig)
{
    IndexerTestConfig config = {0, 0, 0, "default", true};

    IndexerTestFixture fixture;
    fixture.configTest(config);
    BOOST_TEST_MESSAGE(barrelConfig.str());

    Indexer* pIndexer = fixture.getIndexer();
    MockIndexMerger mockIndexMerger(pIndexer, pIndexMergePolicy);

    BarrelsInfo* pBarrelsInfo = pIndexer->getBarrelsInfo();
    int docNumSum = 0;
    BOOST_FOREACH(int docNum, barrelConfig.docNums_)
    {
        if(docNum == 0)
            break;

        BarrelInfo* pNewBarrelInfo = newBarrelInfo(pBarrelsInfo, docNum);
        mockIndexMerger.addToMerge(pBarrelsInfo, pNewBarrelInfo);
        docNumSum += docNum;
    }

    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), barrelConfig.mergedBarrelNum_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), docNumSum);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), docNumSum);
}

/**
 * check function @c IndexMerger::merge using @c OptimizeMerger policy.
 * @p barrelConfig the parameter of barrels to merge
 */
void checkOptimizeMerge(const BarrelConfig& barrelConfig)
{
    IndexerTestConfig config = {0, 0, 0, "default", true};

    IndexerTestFixture fixture;
    fixture.configTest(config);
    BOOST_TEST_MESSAGE(barrelConfig.str());

    Indexer* pIndexer = fixture.getIndexer();

    BarrelsInfo* pBarrelsInfo = pIndexer->getBarrelsInfo();
    int docNumSum = 0;
    BOOST_FOREACH(int docNum, barrelConfig.docNums_)
    {
        if(docNum == 0)
            break;

        newBarrelInfo(pBarrelsInfo, docNum);
        docNumSum += docNum;
    }

    MockIndexMerger mockIndexMerger(pIndexer, new OptimizeMerger(pBarrelsInfo->getBarrelCount()));
    mockIndexMerger.merge(pBarrelsInfo);

    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), barrelConfig.mergedBarrelNum_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), docNumSum);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), docNumSum);
}

BOOST_AUTO_TEST_SUITE( t_IndexMergePolicy )

BOOST_AUTO_TEST_CASE(addToMerge)
{
    const BarrelConfig barrelConfigs[] = {
        {{1}, 1}
        ,{{1,1}, 2}
        ,{{1,1,1}, 1}
        ,{{10,10,10,10}, 2}
        ,{{10,10,10,30,30}, 1}
        ,{{10,10,10,30,30,30}, 2}
        ,{{10,10,10,30,30,90,90}, 1}
        ,{{1,1,1,3,3,9,9,100,1000}, 3}
    };
    const int configNum = sizeof(barrelConfigs)/sizeof(BarrelConfig);

    for(int i=0; i<configNum; ++i)
        checkAddToMerge(new BTMerger, barrelConfigs[i]);
}

BOOST_AUTO_TEST_CASE(optimizeMerge)
{
    const BarrelConfig barrelConfigs[] = {
        {{1}, 1}
        ,{{1,2}, 1}
        ,{{1,3,1}, 1}
        ,{{10,10,10,10}, 1}
        ,{{10,50,20,30,30}, 1}
        ,{{10,100,10,3000,30,30}, 1}
        ,{{1000,10,100,30,30,90,90}, 1}
        ,{{1,1,1,3,3,9,9,100,1000}, 1}
    };
    const int configNum = sizeof(barrelConfigs)/sizeof(BarrelConfig);

    for(int i=0; i<configNum; ++i)
        checkOptimizeMerge(barrelConfigs[i]);
}

BOOST_AUTO_TEST_SUITE_END()


