/**
* @file        IndexMergeManager.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Manage index merging process
*/
#ifndef INDEXMERGER_MANAGER_H
#define INDEXMERGER_MANAGER_H

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <util/concurrent_queue.h>


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

class IndexMergeManager
{
public:
    /**
     * Constructor.
     * @param pIndexer the indexer
     */
    IndexMergeManager(Indexer* pIndexer);

    /**
     * Destructor.
     * If @c isAsync_ is true, it clears current appending
     * merge requests, and block the calling thread
     * until the merge thread finishes its current task.
     */
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
     * Pause current merging activity.
     * Notes: this function only works when @c isAsync_ is true.
     * Notes: if merging thread is removing barrel files currently, which might make query functions return no result,
     *        this function would wait until the end of barrels removal and merged barrel creation.
     */
    void pauseMerge();

    /**
     * Continue the merging activity, which is paused by previous call of @p pauseMerge().
     * Notes: this function only works when @c isAsync_ is true.
     */
    void resumeMerge();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and create a new thread for future merge request.
     * Notes: this function only works when @c isAsync_ is true.
     */
    void waitForMergeFinish();

    /**
     * Whether merging activity is paused.
     * @return true for paused, false for resumed.
     */
    bool isPauseMerge() const { return isPauseMerge_; }

    boost::mutex& getPauseMergeMutex() { return pauseMergeMutex_; }

    boost::condition_variable& getPauseMergeCond() { return pauseMergeCond_; }

private:
    void mergeIndex();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and delete the merge thread.
     * Notes: this function assumes @c isAsync_ is true.
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

private:
    Indexer* pIndexer_;

    BarrelsInfo* pBarrelsInfo_;

    izenelib::util::concurrent_queue<MergeOP> tasks_;

    IndexMerger* pAddMerger_; ///< the merger called when new barrel is added

    boost::thread* pMergeThread_;

    bool isPauseMerge_; ///< whether merge should be paused

    boost::condition_variable pauseMergeCond_; ///< condition variable to pause merge

    boost::mutex pauseMergeMutex_; ///< mutex used for @p isPauseMerge_ and @p pauseMergeCond_

    /**
     * true for merge asynchronously (merge index in a separated thread),
     * false for merge synchronously (single thread for both build index and merge index).
     */
    const bool isAsync_;

    /**
     * mutex used for manage @p pMergeThread_.
     * It's used to avoid concurrent execution of @c waitForMergeFinish()
     * and @c ~IndexMergeManager().
     */
    boost::mutex mergeThreadMutex_;
};

}

NS_IZENELIB_IR_END

#endif
