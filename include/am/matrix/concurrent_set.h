/**
 * @file concurrent_set.h
 * @brief ConcurrentSet is an encapsulation of several set functions,
 *        it could be called concurrently in multi-threads.
 * @author Jun Jiang
 * @date 2011-11-23
 */

#ifndef IZENELIB_AM_CONCURRENT_SET_H
#define IZENELIB_AM_CONCURRENT_SET_H

#include <types.h>
#include <3rdparty/am/stx/btree_set>
#include <util/ThreadModel.h>

#include <utility> // std::pair

NS_IZENELIB_AM_BEGIN

template<typename KeyType,
         typename ContainerType = ::stx::btree_set<KeyType>,
         typename LockType = izenelib::util::ReadWriteLock>
class ConcurrentSet
{
public:
    typedef KeyType key_type;
    typedef ContainerType container_type;
    typedef typename container_type::size_type size_type;

    size_type size() const
    {
        ScopedReadLock lock(lock_);
        return container_.size();
    }

    bool empty() const
    {
        ScopedReadLock lock(lock_);
        return container_.empty();
    }

    std::size_t occupy_size() const
    {
        return sizeof(key_type)*size();
    }

    /** return 1 for @p key exists, 0 for not exists. */
    size_type count(const key_type& key) const
    {
        ScopedReadLock lock(lock_);
        return container_.count(key);
    }

    bool insert(const key_type& key)
    {
        ScopedWriteLock lock(lock_);
        std::pair<iterator, bool> result = container_.insert(key);
        return result.second;
    }

    bool erase(const key_type& key)
    {
        ScopedWriteLock lock(lock_);
        size_type result = container_.erase(key);
        return result ? true : false;
    }

    void swap(container_type& c)
    {
        ScopedWriteLock lock(lock_);
        container_.swap(c);
    }

private:
    typedef izenelib::util::ScopedReadLock<LockType> ScopedReadLock;
    typedef izenelib::util::ScopedWriteLock<LockType> ScopedWriteLock;

    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;

    container_type container_;
    mutable LockType lock_;
};

NS_IZENELIB_AM_END

#endif // IZENELIB_AM_CONCURRENT_SET_H
