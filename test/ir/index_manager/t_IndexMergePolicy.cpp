#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "IndexerTestFixture.h"
#include <ir/index_manager/index/MockIndexMerger.h>
#include <ir/index_manager/index/BTPolicy.h>
#include <ir/index_manager/index/OptimizePolicy.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>

#include <string>
#include <sstream> // std::ostringstream
#include <iostream>

using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;

/**
 * The parameters in testing IndexMergePolicy module.
 */
struct BarrelConfig
{
    enum {
        MAX_BARREL_NUM = 100, ///< maximum barrels number allowed
        END_DOC_NUM = -1 ///< the end value of barrel sequence
    };

    /**
     * doc number in each barrel.
     * value -1 denotes the end of barrel sequence.
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
            if(docNum == END_DOC_NUM)
                break;
            oss << docNum << ", ";
        }
        oss << "), mergedBarrelNum_: " << mergedBarrelNum_ << ")";

        return oss.str();
    }
};

namespace
{
/**
 * barrel parameters.
 * the last number in each line is the barrel number after merge,
 * it would be checked against in @c checkAddToMerge(),
 * while in @c checkOptimizeMerge(), the barrel number after merge is always 1.
 */
const BarrelConfig BARREL_CONFIGS[] = {
    {{1,-1}, 1}
    ,{{1,1,-1}, 2}
    ,{{1,1,1,-1}, 1}
    ,{{10,10,10,10,-1}, 2} // combine to {10,10,10}, {10}
    ,{{10,10,10,30,30,-1}, 1}
    ,{{10,10,10,30,30,30,-1}, 2}
    ,{{10,10,10,30,30,90,90,-1}, 1}
    ,{{10,50,20,30,30,-1}, 1}
    ,{{10,100,20,3000,30,50,25,-1}, 1}
    ,{{1000,10,100,30,30,90,90,-1}, 3} // combine to {1000}, {10}, {100,30,30,90,90}
    ,{{1,1,1,3,3,9,9,100,1000,-1}, 3} // combine to {1,1,1,3,3,9,9}, {100}, {1000}
    ,{{1,1,3,100,3,9,9,1,1000,-1}, 2} // combine to {1,1,3,100,3,9,9,1}, {1000}
    ,{{1,100,1,1,3,1000,3,9,9,3,-1}, 2} // combine to {1,100,1,1}, {3,1000,3,9,9,3}
    ,{{10,5,1,2,5,-1}, 5} // no combine
    ,{{10,5,1,2,5,6,-1}, 2} // combine to {10}, {5,1,2,5,6}
    ,{{10,5,1,2,5,6,5,-1}, 3} // combine to {10}, {5,1,2,5,6}, {5}
    // check empty barrel below
    ,{{0,0,0,-1}, 1}
    ,{{1,0,0,-1}, 1}
    ,{{0,1,0,-1}, 1}
    ,{{0,0,1,-1}, 1}
    ,{{1,1,0,-1}, 1}
    ,{{1,0,1,-1}, 1}
    ,{{0,1,1,-1}, 1}
    ,{{0,1,2,-1}, 1}
    ,{{0,3,1,2,-1}, 1}
    ,{{1,0,3,2,-1}, 1}
    ,{{3,1,0,2,-1}, 2} // combine to {3}, {1,0,2}
    ,{{2,0,1,3,-1}, 2} // combine to {2,0,1}, {3}
    ,{{10,5,1,2,5,0,-1}, 2} // combine to {10}, {5,1,2,5,0}
    ,{{10,5,1,0,5,2,-1}, 2} // combine to {10}, {5,1,0,5,2}
};

const int CONFIG_NUM = sizeof(BARREL_CONFIGS)/sizeof(BarrelConfig);
}

/**
 * create a new @c BarrelInfo.
 * @param pBarrelsInfo the @c BarrelsInfo instance
 * @param docNum the number of doc contained in new @c BarrelInfo
 * @return the new created instance
 */
BarrelInfo* newBarrelInfo(BarrelsInfo* pBarrelsInfo, int docNum, IndexLevel indexLevel)
{
    BarrelInfo* pNewBarrelInfo = new BarrelInfo(pBarrelsInfo->newBarrel(), docNum, indexLevel);
    pNewBarrelInfo->baseDocIDMap[IndexerTestFixture::COLLECTION_ID] = pBarrelsInfo->maxDocId() + 1;
    pNewBarrelInfo->maxDocId = pBarrelsInfo->maxDocId() + docNum;
    pNewBarrelInfo->setSearchable(true);
    pNewBarrelInfo->setWriter(NULL);
    pBarrelsInfo->addBarrel(pNewBarrelInfo);
    pBarrelsInfo->updateMaxDoc(pNewBarrelInfo->maxDocId);

    return pNewBarrelInfo;
}

/**
 * check function @c IndexMerger::addToMerge using @p pIndexMergePolicy.
 * @p pIndexMergePolicy the instance of merge policy
 * @p barrelConfig the parameter of barrels to merge
 */
void checkAddToMerge(IndexMergePolicy* pIndexMergePolicy, const BarrelConfig& barrelConfig, IndexLevel indexLevel)
{
    IndexerTestConfig config = {0, 0, 0, indexLevel, "default", true};

    IndexerTestFixture fixture;
    fixture.configTest(config);
    BOOST_TEST_MESSAGE(barrelConfig.str());

    Indexer* pIndexer = fixture.getIndexer();
    MockIndexMerger mockIndexMerger(pIndexer, pIndexMergePolicy);

    BarrelsInfo* pBarrelsInfo = pIndexer->getBarrelsInfo();
    int docNumSum = 0;
    BOOST_FOREACH(int docNum, barrelConfig.docNums_)
    {
        if(docNum == BarrelConfig::END_DOC_NUM)
            break;

        BarrelInfo* pNewBarrelInfo = newBarrelInfo(pBarrelsInfo, docNum, indexLevel);
        mockIndexMerger.addToMerge(pNewBarrelInfo);
        docNumSum += docNum;
    }

    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), barrelConfig.mergedBarrelNum_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), static_cast<unsigned int>(docNumSum));
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), docNumSum);
}

/**
 * check function @c IndexMerger::merge using @c OptimizePolicy policy.
 * @p barrelConfig the parameter of barrels to merge
 * @note @p barrelConfig.mergedBarrelNum_ is ignored, as this function always check against 1.
 */
void checkOptimizeMerge(const BarrelConfig& barrelConfig, IndexLevel indexLevel)
{
    IndexerTestConfig config = {0, 0, 0, indexLevel, "default", true};

    IndexerTestFixture fixture;
    fixture.configTest(config);

    BarrelConfig newBarrelConfig(barrelConfig);
    newBarrelConfig.mergedBarrelNum_ = 1; // barrel number after optimize should be 1
    BOOST_TEST_MESSAGE(newBarrelConfig.str());

    Indexer* pIndexer = fixture.getIndexer();

    BarrelsInfo* pBarrelsInfo = pIndexer->getBarrelsInfo();
    int docNumSum = 0;
    BOOST_FOREACH(int docNum, newBarrelConfig.docNums_)
    {
        if(docNum == BarrelConfig::END_DOC_NUM)
            break;

        newBarrelInfo(pBarrelsInfo, docNum, indexLevel);
        docNumSum += docNum;
    }

    MockIndexMerger mockIndexMerger(pIndexer, new OptimizePolicy(pBarrelsInfo->getBarrelCount()));
    mockIndexMerger.mergeBarrels();

    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), newBarrelConfig.mergedBarrelNum_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), static_cast<unsigned int>(docNumSum));
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), docNumSum);
}

BOOST_AUTO_TEST_SUITE( t_IndexMergePolicy )

BOOST_AUTO_TEST_CASE(addToMerge)
{
    IndexLevel indexl = WORDLEVEL;
    for(int i=0; i<CONFIG_NUM; ++i)
        checkAddToMerge(new BTPolicy, BARREL_CONFIGS[i], indexl);
}

BOOST_AUTO_TEST_CASE(optimizeMerge)
{
    IndexLevel indexl = WORDLEVEL;
    for(int i=0; i<CONFIG_NUM; ++i)
        checkOptimizeMerge(BARREL_CONFIGS[i], indexl);
}

BOOST_AUTO_TEST_SUITE_END()


