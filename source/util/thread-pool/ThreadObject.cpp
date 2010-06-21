/// @file ThreadObject.h
/// @brief header file of ThreadObject classe
/// @author KyoungHoon
/// @date 2008-09-30

#include <util/thread-pool/ThreadObject.h>
#include <util/thread-pool/ThreadObjectPool.h>

#include <iostream>

NS_IZENELIB_UTIL_BEGIN
namespace thread_pool {

ThreadObject::ThreadObject(void) : thread_(NULL), threadObjectPool_(NULL)
{
    status_ = UNREADY;
    isRepeat_ = true;
    isThreadTerminated_ = false;
}

ThreadObject::~ThreadObject(void)
{
    destroy();
}

void ThreadObject::destroy(void)
{
    if( thread_ )
    {
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
}

void ThreadObject::setThreadObjectPool(ThreadObjectPool* threadObjectPool)
{
    threadObjectPool_ = threadObjectPool;
}

boost::thread& ThreadObject::createThread(bool isRepeat)
{
    if( isRepeat == false )
        isRepeat_ = isRepeat;

    thread_ = new boost::thread( boost::bind( &ThreadObject::threadFunction, this ) );
    return (*thread_);
}

void ThreadObject::threadFunction(void)
{
    while( true )
    {
        boost::mutex::scoped_lock lock( conditionMutex_ );

        /// change current status is READY
        setStatus( READY );

        /// wait for condition
        /// locking of the conditionMutex_ is released in wait() function
        condition_.wait( lock );

        if( status_ == STOP )
        {
            shutdown();
            break;
        }

        /// change current status is RUNNING
        setStatus( RUNNING );
        
        this->start();

        if( isRepeat_ == false )
        {
            shutdown();
            break;
        }

        jobDone();
        
    } // end of while
}

bool ThreadObject::run(void)
{
    /// wait until current status to be READY
    if( waitUntilReady() == false )
        return false;

    /// blocked until threadFunction waits for condition
    boost::mutex::scoped_lock lock( conditionMutex_ );
    condition_.notify_one();

    return true;
}

bool ThreadObject::finish(void)
{
    /// wait until current status to be READY
    if( waitUntilReady() == false )
        return false;

    /// blocked until threadFunction waits for condition
    boost::mutex::scoped_lock lock( conditionMutex_ );
    setStatus( STOP );
    condition_.notify_one();
    return true;
}

void ThreadObject::jobDone(void)
{
    if( threadObjectPool_ )
    {
        threadObjectPool_->restore( this );
    }
}

void ThreadObject::shutdown(void)
{
    isThreadTerminated_ = true;

    if( threadObjectPool_ )
    {
        threadObjectPool_->finish( this );
    }
}

void ThreadObject::setStatus(Status status)
{
    boost::mutex::scoped_lock lock( statusMutex_ );

    status_ = status;

    if( status == READY )
        condition_.notify_one();
}

bool ThreadObject::waitUntilReady(void)
{
    boost::mutex::scoped_lock lock( statusMutex_ );

    if( status_ != READY )
    {
        if( isThreadTerminated_ /* isRepeat_ == false && */ )
        {
            return false;
        }

        /// wait until current status to be READY
        /// locking of the statusMutex_ is released in wait() function
        condition_.wait( lock );
    }

    return true;
}

}
NS_IZENELIB_UTIL_END// namespace
