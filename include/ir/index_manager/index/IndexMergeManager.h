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
#include <ir/index_manager/index/BarrelInfo.h>

#include <util/concurrent_queue.h>


using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

enum MergeOPType
{
    ADD_BARREL,
    OPTIMIZE_ALL,
    EXIT_MERGE
};

struct MergeOP
{
    MergeOPType opType;
    BarrelInfo* pBarrelInfo;

    MergeOP(MergeOPType type = ADD_BARREL) : opType(type), pBarrelInfo(NULL) {}
};

class IndexMerger;
class OptimizeMerger;

class IndexMergeManager
{
public:
    /**
     * Constructor.
     * @param pIndexer the indexer
     */
    IndexMergeManager(Indexer* pIndexer);

    ~IndexMergeManager();
public:
    /**
     * add an index barrel to merge
     * @param pBarrelInfo the barrel need to merge
     */
    void addToMerge(BarrelInfo* pBarrelInfo);

    /**
     * Clear current appending merge requests, and merge all barrels into one.
     */
    void optimizeIndex();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and create a new thread for future merge request.
     * Notes: this function only works when @p isAsyncMerge is true.
     */
    void waitForMergeFinish();

private:
    void mergeIndex();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and delete the merge thread.
     * Notes: this function only works when @p isAsyncMerge is true.
     */
    void joinMergeThread();

    /**
     * Implementation to merge all barrels into one.
     */
    void optimizeIndexImpl();

    /**
     * Create the merge thread and run it.
     */
    void run();

    /**
     * Clear current appending merge requests, and block the calling thread
     * until the merge thread finishes its current task.
     * Notes: this function only works when @p isAsyncMerge is true.
     */
    void stop();

private:
    Indexer* pIndexer_;

    BarrelsInfo* pBarrelsInfo_;

    izenelib::util::concurrent_queue<MergeOP> tasks_;

    IndexMerger* pAddMerger_; ///< the merger called when new barrel is added

    OptimizeMerger* pOptimizeMerger_; ///< the merger called when to optimize all barrels

    boost::thread* pMergeThread_;
};

}

NS_IZENELIB_IR_END

#endif

