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

class MergePolicy
{
public:
    MergePolicy(BarrelsInfo* pBarrelsInfo, Directory* pDirectory)
        :pBarrelsInfo_(pBarrelsInfo)
        ,colID_(1)
    {
        pMerger_ = new MockMerger(pBarrelsInfo,pDirectory);
        pMerger_->merge();
        currMaxDocID_ = pBarrelsInfo->maxDocId();
    }
    ~MergePolicy()
    {
        delete pMerger_;
    }

public:
    void addToMerge(int docNumInBarrel)
    {
        BarrelInfo* pCurBarrelInfo = new BarrelInfo(pBarrelsInfo_->newBarrel(),docNumInBarrel);
        docid_t currBaseDocID = currMaxDocID_ + 1;
        currMaxDocID_ = currBaseDocID + docNumInBarrel - 1;
        pCurBarrelInfo->baseDocIDMap[colID_] = currBaseDocID;
        pCurBarrelInfo->maxDocId = currMaxDocID_;
        pCurBarrelInfo->setSearchable(true);
        pCurBarrelInfo->setWriter(NULL);
        pBarrelsInfo_->addBarrel(pCurBarrelInfo,false);
        pBarrelsInfo_->updateMaxDoc(currMaxDocID_);

        pMerger_->addToMerge(pCurBarrelInfo);
    }

private:
    BarrelsInfo* pBarrelsInfo_;
    MockMerger* pMerger_;
    collectionid_t colID_;
    docid_t currMaxDocID_;
};

BOOST_AUTO_TEST_SUITE( t_MergePolicy )


BOOST_AUTO_TEST_CASE(merge)
{
    bfs::path path(bfs::path(".") /"index");
    FSDirectory* pDirectory = new FSDirectory(path.string(),true);
    BarrelsInfo* pBarrelsInfo = new BarrelsInfo;
    pBarrelsInfo->read(pDirectory);
    MergePolicy merger(pBarrelsInfo, pDirectory);

    int barrels[] = {
        100000,
        10000,
        10000,
        4000,
        10000
    };
    int numBarrels = sizeof(barrels)/sizeof(int);

    for(int i = 0; i < numBarrels; ++i)
    {
        merger.addToMerge(barrels[i]);
    }

    pBarrelsInfo->write(pDirectory);

    delete pDirectory;
    delete pBarrelsInfo;
}

BOOST_AUTO_TEST_SUITE_END()


