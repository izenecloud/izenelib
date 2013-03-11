#ifndef AM_CONCEPT_THREADMODEL_H
#define AM_CONCEPT_THREADMODEL_H

#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <types.h>

NS_IZENELIB_UTIL_BEGIN

class NullLock
{
public:
    void lock_shared() {}

    void unlock_shared() {}

    void lock() {}

    void unlock() {}
};

class ReadWriteLock : private boost::noncopyable
{
private:
    boost::shared_mutex rwMutex_;
public:
    explicit ReadWriteLock()
    {}

    ~ReadWriteLock()
    {}

    /**
     * @ brief Get entity mutex.
     */
    boost::shared_mutex& get_entity()
    {
        return rwMutex_;
    }

    /**
     * @ brief Attempts to get the read lock.
     */
    void lock_shared()
    {
        rwMutex_.lock_shared();
    }
    /**
     *  @ brief Attempts to get the write lock.
     */
    void lock()
    {
        rwMutex_.lock();
    }
    /**
     *  @ brief Attempts to release the  read lock .
     */
    void unlock_shared()
    {
        rwMutex_.unlock_shared();
    }
    /**
     * @ brief Attempts to release the write lock.
     */
    void unlock()
    {
        rwMutex_.unlock();
    }
};

class RecursiveLock : private boost::noncopyable
{
private:
    boost::recursive_mutex recurMutex_;
public:
    explicit RecursiveLock()
    {}

    ~RecursiveLock()
    {
    }

    /**
     * @ brief Attempts to get the read lock.
     */
    void lock()
    {
        recurMutex_.lock();
    }
    /**
     *  @ brief Attempts to get the write lock.
     */
    void lock_shared()
    {
        recurMutex_.lock();
    }
    /**
     *  @ brief Attempts to release the  read lock .
     */
    void unlock()
    {
        recurMutex_.unlock();
    }
    /**
     * @ brief Attempts to release the write lock.
     */
    void unlock_shared()
    {
        recurMutex_.unlock();
    }
};


template<class LockType>
class ScopedReadLock
{
    LockType& lock_;
public:
    explicit ScopedReadLock( LockType& lock):lock_(lock)
    {
        lock_.lock_shared();
    }
    ~ScopedReadLock()
    {
        lock_.unlock_shared();
    }
private:
    DISALLOW_COPY_AND_ASSIGN(ScopedReadLock);
};

template<class LockType>
class ScopedWriteLock
{
    LockType& lock_;
public:
    explicit ScopedWriteLock( LockType& lock):lock_(lock)
    {
        lock_.lock();
    }
    ~ScopedWriteLock()
    {
        lock_.unlock();
    }
private:
    DISALLOW_COPY_AND_ASSIGN(ScopedWriteLock);
};

NS_IZENELIB_UTIL_END

namespace boost
{
typedef boost::detail::try_lock_wrapper<boost::shared_mutex> shared_scoped_try_lock;
}

#endif
