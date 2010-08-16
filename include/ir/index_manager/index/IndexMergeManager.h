/**
* @file        IndexMergeManager.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Manage index merging process
*/
#ifndef INDEXMERGER_MANAGER_H
#define INDEXMERGER_MANAGER_H

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <map>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <util/concurrent_queue.h>


using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

enum MergeOPType
{
    ONLINE,
    FINISH,
    OFFLINE,
    NOOP
};

struct MergeOP
{
    MergeOPType opType;
    BarrelInfo* pBarrelInfo;
};

class IndexMergeManager
{
public:
    IndexMergeManager(Indexer* pIndexer);

    ~IndexMergeManager();
public:
    void run();

    void triggerMerge(BarrelInfo* pBarrelInfo);

    void optimizeIndex();

    void stop();
private:
    void mergeIndex();

private:
    Indexer* pIndexer_;

    BarrelsInfo* pBarrelsInfo_;

    izenelib::util::concurrent_queue<MergeOP> tasks_;

    std::map<MergeOPType, IndexMerger*> indexMergers_;

    boost::shared_ptr<boost::thread> mergethread_;
};

}

NS_IZENELIB_IR_END

#endif

