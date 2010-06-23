/// @file ThreadObjectPool.cpp
/// @brief cpp file of ThreadObjectPool classe
/// @author KyoungHoon
/// @date 2008-10-06

#include <util/thread-pool/ThreadObjectPool.h>

#include <unistd.h>

NS_IZENELIB_UTIL_BEGIN
namespace thread_pool {

ThreadObjectPool::ThreadObjectPool(void)
{
    nThreadObject_ = 0;
    nIdleThreadObject_ = 0;
    nExpiredThreadObject_ = 0;
    bIsAllThreadShutdowned_ = false;
}

ThreadObjectPool::~ThreadObjectPool(void)
{
    shutdownAllThread();
}

void ThreadObjectPool::add(const ThreadObject* threadObject)
{
    /// increase number of total thread objects
    nThreadObject_++;
    restore( threadObject );
}

bool ThreadObjectPool::get(ThreadObject*& threadObject)
{
    /// lock
    boost::mutex::scoped_lock firstLock( shutdownMutex_ );
    boost::mutex::scoped_lock secondLock( idleThreadObjectQueueMutex_ );
    if( idleThreadObjectQueue_.empty() )
        return false;

    /// pop one thread object
    threadObject = idleThreadObjectQueue_.front();
    idleThreadObjectQueue_.pop_front();

    /// decrease number of idle thread objects
    nIdleThreadObject_--;

    return true;
}

void ThreadObjectPool::restore(const ThreadObject* threadObject)
{
    /// lock
    boost::mutex::scoped_lock lock( idleThreadObjectQueueMutex_ );

    idleThreadObjectQueue_.push_back( (ThreadObject*) threadObject );
    /// increase number of idle thread objects
    nIdleThreadObject_++;
}

void ThreadObjectPool::finish(const ThreadObject* threadObject)
{
    /// lock
    boost::mutex::scoped_lock lock( expiredThreadObjectQueueMutex_ );

    expiredThreadObjectQueue_.push_back( (ThreadObject*) threadObject );
    /// increase number of expired thread objects
    nExpiredThreadObject_++;
}

void ThreadObjectPool::shutdownAllThread(void)
{
    int nCount = 0;
    ThreadObject* threadObject = NULL;

    /// lock
    boost::mutex::scoped_lock lock( shutdownMutex_ );
    /// get() function is blocked

    if( nThreadObject_ == 0 ||
        bIsAllThreadShutdowned_ == true )
        return;

    while( nCount < nThreadObject_ )
    {
        /// lock
        idleThreadObjectQueueMutex_.lock();

        if( idleThreadObjectQueue_.empty() == true )
        {
            /// release lock
            idleThreadObjectQueueMutex_.unlock();
            usleep( 100 );
            continue;
        }

        /// pop one thread object from idle thread object queue
        threadObject = idleThreadObjectQueue_.front();
        idleThreadObjectQueue_.pop_front();

        /// decrease number of idle thread objects
        nIdleThreadObject_--;

        /// release lock
        idleThreadObjectQueueMutex_.unlock();

        if( threadObject )
        {
            /// thread object's threadFunction() finished.
            threadObject->finish();
            nCount++;
            delete threadObject;
            threadObject = NULL;
        }
    }

    while( true )
    {
        expiredThreadObjectQueueMutex_.lock();

        if( nExpiredThreadObject_ < nThreadObject_ )
        {
            expiredThreadObjectQueueMutex_.unlock();
            usleep( 100 );
            continue;
        }
        else
        {
            expiredThreadObjectQueueMutex_.unlock();
            bIsAllThreadShutdowned_ = true;
            break;
        }
    }
}

}
NS_IZENELIB_UTIL_END// namespace
