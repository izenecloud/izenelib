#ifndef INDEXMERGERAGENT_H
#define INDEXMERGERAGENT_H

#include <boost/thread.hpp>

#include <string>
#include <vector>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/BarrelInfo.h>


using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class IndexMergerAgent
{
public:
    IndexMergerAgent(Indexer* pIndexer);

    ~IndexMergerAgent();
public:
    void triggerMerge();

    string getCurrentBarrelName();	

    void mergeBarrelsInfo();
private:
    void mergeIndex();

private:
    Indexer* pIndexer_;

    IndexMerger* pIndexMerger_;

    BarrelsInfo* pBarrelsInfo_;

    int32_t nBarrelCounter_;

    vector<BarrelInfo*> waitingBarrels_;

    boost::mutex mutex_;
};

}

NS_IZENELIB_IR_END

#endif
