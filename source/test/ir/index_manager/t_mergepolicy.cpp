#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>
#include <map>
#include <vector>

#include <ir/index_manager/index/MockMerger.h>
#include <ir/index_manager/store/FSDirectory.h>


using namespace std;
using namespace boost;
namespace bfs = boost::filesystem;

using namespace izenelib::ir::indexmanager;

BOOST_AUTO_TEST_SUITE( t_MergePolicy )

BOOST_AUTO_TEST_CASE(merge)
{
    collectionid_t colID = 1;

    bfs::path path(bfs::path(".") /"index");
    FSDirectory* pDirectory = new FSDirectory(path.string(),true);
    BarrelsInfo* pBarrelsInfo = new BarrelsInfo;
    pBarrelsInfo->read(pDirectory);

    MockMerger* pMerger = new MockMerger(pBarrelsInfo,pDirectory);
    pMerger->merge();

    docid_t currMaxDocID = pBarrelsInfo->maxDocId();

    int numBarrels = 10;

    for(int i = 0; i < numBarrels; ++i)
    {
        int docNum = 1000;
        BarrelInfo* pCurBarrelInfo = new BarrelInfo(pBarrelsInfo->newBarrel(),docNum);
        docid_t currBaseDocID = currMaxDocID + 1;
        currMaxDocID = currBaseDocID + docNum - 1;
        pCurBarrelInfo->baseDocIDMap[colID] = currBaseDocID;
        pCurBarrelInfo->maxDocId = currMaxDocID;
        pCurBarrelInfo->setSearchable(true);
        pCurBarrelInfo->setWriter(NULL);
        pBarrelsInfo->addBarrel(pCurBarrelInfo,false);
        pBarrelsInfo->updateMaxDoc(currMaxDocID);

        pMerger->addToMerge(pCurBarrelInfo);
    }
    pBarrelsInfo->write(pDirectory);

    delete pDirectory;
    delete pBarrelsInfo;
    delete pMerger;
}

BOOST_AUTO_TEST_SUITE_END()


