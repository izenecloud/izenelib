/**
* @file        IndexMergeManager.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Manage index merging process
*/
#ifndef INDEXMERGER_MANAGER_H
#define INDEXMERGER_MANAGER_H

#include <boost/thread.hpp>

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
    OFFLINE,
    NOOP
};

struct MergeOP
{
    MergeOPType opType;
    BarrelInfo* pBarrelInfo;

    MergeOP(MergeOPType type = ONLINE) : opType(type), pBarrelInfo(NULL) {}
};

class IndexMergeManager
{
public:
    IndexMergeManager(Indexer* pIndexer);

    ~IndexMergeManager();
public:
    void run();

    void triggerMerge(BarrelInfo* pBarrelInfo);

    /**
     * Clear current appending merge requests, and merge all barrels into one.
     */
    void optimizeIndex();

    /**
     * Clear current appending merge requests, and block the calling thread
     * until the merge thread finishes its current task.
     * Notes: this function only works when \p run() is called beforehand.
     */
    void stop();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and create a new thread for future merge request.
     * Notes: this function only works when \p run() is called beforehand.
     */
    void waitForMergeFinish();

private:
    void mergeIndex();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and delete the merge thread.
     * Notes: this function only works when \p run() is called beforehand.
     */
    void joinMergeThread();

private:
    Indexer* pIndexer_;

    BarrelsInfo* pBarrelsInfo_;

    izenelib::util::concurrent_queue<MergeOP> tasks_;

    std::map<MergeOPType, IndexMerger*> indexMergers_;

    boost::thread* pMergeThread_;
};

}

NS_IZENELIB_IR_END

#endif

