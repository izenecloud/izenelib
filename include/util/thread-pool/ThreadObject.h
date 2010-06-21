/// @file ThreadObject.h
/// @brief header file of ThreadObject classe
/// @author KyoungHoon
/// @date 2008-09-30

#if !defined(_THREAD_OBJECT_)
#define _THREAD_OBJECT_

#include <types.h>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

NS_IZENELIB_UTIL_BEGIN

namespace thread_pool {

class ThreadObjectPool;

///
/// @brief thread class
///
class ThreadObject
{
private:
    boost::thread* thread_;
    ThreadObjectPool* threadObjectPool_;

    /// for concurrency
    boost::mutex conditionMutex_;
    boost::mutex statusMutex_;
    boost::condition condition_;

    bool isRepeat_;
    bool isThreadTerminated_;

    enum Status
    {
        UNREADY = 1,
        READY   = 2,
        RUNNING = 3,
        STOP    = 4
    } status_;

public:
    /// @brief a constructor
    ThreadObject(void);

    /// @brief a destructor
    virtual ~ThreadObject(void);

    void setThreadObjectPool(ThreadObjectPool* threadObjectPool);

    ///
    /// @brief creates thread. threadFunction() runs as thread
    /// @param isRepeat if value is true, threadFunction() work repeatedly when run() function is called.
    /// @return thread handler
    ///
    boost::thread& createThread(bool isRepeat);

    ///
    /// @brief run thread function.
    /// @return true - function successfully, false - thread already terminated
    ///
    bool run(void);

    ///
    /// @brief finish the thread
    /// @return true - function successfully, false - thread already terminated
    ///
    bool finish(void);

private:
    /// @brief destroy the object
    void destroy(void);

    /// @brief thread function
    void threadFunction(void);

    /// @brief change the current status to given status
    void setStatus(Status status);

    /// @brief wait until current status to be READY
    bool waitUntilReady(void);

    /// @brief restore itself to ThreadObjectPool
    void jobDone(void);

    /// @brief shutdown thread
    void shutdown(void);

    ///
    /// @brief pure virtual function. ths function called in threadFunction()
    /// inherited this class must has to realize this function
    ///
    virtual void start(void) = 0;
};

}
NS_IZENELIB_UTIL_END// namespace

#endif // _THREAD_OBJECT_
