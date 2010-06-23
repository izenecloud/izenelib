/// @file ThreadObjectPool.h
/// @brief header file of ThreadObjectPool classe
/// @author KyoungHoon
/// @date 2008-10-06

#if !defined(_THREAD_OBJECT_POOL_)
#define _THREAD_OBJECT_POOL_

#include <util/thread-pool/ThreadObject.h>
#include <boost/thread/mutex.hpp>
#include <deque>

NS_IZENELIB_UTIL_BEGIN

namespace thread_pool {

///
/// @brief thread object pool class
///
class ThreadObjectPool
{
private:
    int nThreadObject_;
    int nIdleThreadObject_;
    int nExpiredThreadObject_;
    bool bIsAllThreadShutdowned_;

    /// idle thread object queue
    std::deque<ThreadObject*> idleThreadObjectQueue_;
    boost::mutex idleThreadObjectQueueMutex_;

    /// expired thread object queue
    std::deque<ThreadObject*> expiredThreadObjectQueue_;
    boost::mutex expiredThreadObjectQueueMutex_;

    boost::mutex shutdownMutex_;

public:
    /// @brief a constructor
    ThreadObjectPool(void);

    /// @brief a destructor
    ~ThreadObjectPool(void);
  
    ///
    /// @brief add given thread object into idle thread object queue
    /// @param threadObject address of thread object
    /// 
    void add(const ThreadObject* threadObject);

    ///
    /// @brief get one idle thread object from idle thread object queue
    /// @param threadObject variable to write address of thread object
    /// @ return true - function successfully, false - function failed.
    ///
    bool get(ThreadObject*& threadObject);

    ///
    /// @brief restore given thread object into idle thread object queue
    /// @param threadObject address of thread object
    /// 
    void restore(const ThreadObject* threadObject);

    ///
    /// @brief push given thread object into expired thread object queue
    /// @param threadObject address of expried thread object
    ///
    void finish(const ThreadObject* threadObject);

    ///
    /// @brief shutdown all thread object and wait until all job done
    ///
    void shutdownAllThread(void);
};

}

NS_IZENELIB_UTIL_END// namespace

#endif //_THREAD_OBJECT_POOL_
