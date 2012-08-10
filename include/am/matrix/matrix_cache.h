/**
 * @file matrix_cache.h
 * @brief MatrixCache is an in-memory matrix, it could be called concurrently in multi-threads.
 * @author Jun Jiang
 * @date 2011-11-23
 */

#ifndef IZENELIB_AM_MATRIX_CACHE_H
#define IZENELIB_AM_MATRIX_CACHE_H

#include <types.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/am/stx/btree_set>
#include <util/ThreadModel.h>

#include <boost/shared_ptr.hpp>
#include <cassert>
#include <iostream>

NS_IZENELIB_AM_BEGIN

/**
 * @c MatrixCache is a map from @p KeyType to @p RowType,
 * @p RowType is a map from column key to element,
 * @c RowType::size() is prerequisite to count the total number of entries in the matrix.
 */
template<typename KeyType,
         typename RowType,
         typename ContainerType = ::rde::hash_map<KeyType, boost::shared_ptr<RowType> >,
         typename LockType = izenelib::util::ReadWriteLock>
class MatrixCache
{
public:
    typedef KeyType key_type;
    typedef RowType row_type;
    typedef typename RowType::key_type col_type;
    typedef typename RowType::mapped_type elem_type;
    typedef ContainerType container_type;

    enum
    {
        ELEM_SIZE = sizeof(col_type) + sizeof(elem_type),
        ROW_SIZE = sizeof(typename container_type::key_type) +
                   sizeof(typename container_type::mapped_type) +
                   sizeof(RowType),
        FLAG_SIZE = sizeof(key_type)
    };

    MatrixCache() : elemCount_(0) {}

    std::size_t size() const
    {
        ScopedReadLock lock(lock_);
        return container_.size();
    }

    bool empty() const
    {
        ScopedReadLock lock(lock_);
        return container_.empty();
    }

    std::size_t elem_count() const
    {
        return elemCount_;
    }

    boost::shared_ptr<RowType> get(const key_type& key) const
    {
        ScopedReadLock lock(lock_);

        boost::shared_ptr<RowType> row;
        const_iterator it = container_.find(key);
        if (it != container_.end())
        {
            row = it->second;
        }

        return row;
    }

    bool insert(const key_type& key, boost::shared_ptr<RowType> row)
    {
        assert(row);
        ScopedWriteLock lock(lock_);

        value_type pairValue(key, row);
        if (container_.insert(pairValue).second)
        {
            elemCount_ += row->size();
            return true;
        }
        return false;
    }

    void update(const key_type& key, boost::shared_ptr<RowType> row)
    {
        assert(row);
        ScopedWriteLock lock(lock_);

        boost::shared_ptr<RowType>& old = container_[key];
        if (old)
        {
            assert(elemCount_ >= old->size());
            elemCount_ -= old->size();
        }
        old = row;
        elemCount_ += old->size();
        dirtyFlags_.insert(key);
    }

    bool update_elem(const key_type& x, const col_type& y, const elem_type& elem)
    {
        ScopedWriteLock lock(lock_);

        iterator it = container_.find(x);
        if (it == container_.end())
            return false;

        boost::shared_ptr<RowType> row = it->second;
        typename row_type::iterator cit = row->find(y);
        if(cit == row->end())
        {
            (*row)[y] = elem;
            ++elemCount_;
        }
        else
        {
            cit->second = elem;
        }
        dirtyFlags_.insert(x);

        return true;
    }

    /**
     * @param key the row to read
     * @param func the function @c func(const RowType&) would be called with the row as argument
     * @return true for success, false for @p key is not found
     */
    template <typename Func>
    bool read_with_func(const key_type& key, Func func) const
    {
        ScopedReadLock lock(lock_);

        const_iterator it = container_.find(key);
        if (it == container_.end())
            return false;

        const RowType& crow(*it->second);
        func(crow);
        return true;
    }

    /**
     * @param key the row to update
     * @param func the function @c func(RowType&) would be called with the row as argument
     * @return true for success, false for @p key is not found
     */
    template <typename Func>
    bool update_with_func(const key_type& key, Func func)
    {
        ScopedWriteLock lock(lock_);

        iterator it = container_.find(key);
        if (it == container_.end())
            return false;

        boost::shared_ptr<RowType> row = it->second;
        const int oldSize = row->size();

        func(*row);

        const int newSize = row->size();
        elemCount_ += newSize - oldSize;
        dirtyFlags_.insert(key);

        return true;
    }

    /**
     * erase a row.
     * @param[in] key the row to erase
     * @param[out] isDirty whether the row is dirty
     * @return the row instance,
     *         if @p key is not found, the return value would hold @c NULL.
     */
    boost::shared_ptr<RowType> erase(const key_type& key, bool& isDirty)
    {
        ScopedWriteLock lock(lock_);

        boost::shared_ptr<RowType> row;
        isDirty = false;

        iterator it = container_.find(key);
        if (it != container_.end())
        {
            row = it->second;
            container_.erase(it);

            assert(elemCount_ >= row->size());
            elemCount_ -= row->size();

            isDirty = dirtyFlags_.erase(key) ? true : false;
        }

        return row;
    }

    /**
     * find a row whose flag is dirty, and reset its flag to not dirty.
     * @param func the function @c func(key, const RowType&) would be called
     *             with the key and row whose flag is reset
     * @return true for one row's dirty flag is reset,
     *         false for no flag is dirty.
     */
    template <typename Func>
    bool reset_dirty_flag(Func func)
    {
        ScopedWriteLock lock(lock_);

        typename DirtyFlags::iterator flagIt = dirtyFlags_.begin();
        if (flagIt == dirtyFlags_.end())
            return false;

        const key_type key = *flagIt;
        const_iterator rowIt = container_.find(key);
        if (rowIt != container_.end())
        {
            const RowType& crow(*rowIt->second);
            func(key, crow);
        }

        dirtyFlags_.erase(flagIt);
        return true;
    }

    void clear()
    {
        ScopedWriteLock lock(lock_);
        container_.clear();
        elemCount_ = 0;
        dirtyFlags_.clear();
    }

    std::size_t occupy_size() const
    {
        return ELEM_SIZE * elemCount_ +
               ROW_SIZE * size() +
               FLAG_SIZE * dirtyFlags_.size();
    }

    void print(std::ostream& ostream) const
    {
        ostream << "ELEM_SIZE[" << ELEM_SIZE << "]*elem[" << elemCount_
                << "] + ROW_SIZE[" << ROW_SIZE << "]*row[" << size()
                << "] + FLAG_SIZE[" << FLAG_SIZE << "]*flag[" << dirtyFlags_.size()
                << "] => MatrixCache[" << occupy_size() << "]";
    }

private:
    typedef izenelib::util::ScopedReadLock<LockType> ScopedReadLock;
    typedef izenelib::util::ScopedWriteLock<LockType> ScopedWriteLock;

    typedef typename container_type::value_type value_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;

    container_type container_;
    std::size_t elemCount_;
    mutable LockType lock_;

    typedef ::stx::btree_set<key_type> DirtyFlags;
    DirtyFlags dirtyFlags_;
};

template<typename KeyType, typename RowType, typename ContainerType, typename LockType>
std::ostream& operator<<(
    std::ostream& out,
    const MatrixCache<KeyType, RowType, ContainerType, LockType>& matrix
)
{
    matrix.print(out);
    return out;
}

NS_IZENELIB_AM_END

#endif // IZENELIB_AM_MATRIX_CACHE_H
