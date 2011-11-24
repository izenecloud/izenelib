/**
 * @file concurrent_matrix.h
 * @brief ConcurrentMatrix is an in-memory matrix, it could be called concurrently in multi-threads.
 * @author Jun Jiang
 * @date 2011-11-23
 */

#ifndef IZENELIB_AM_CONCURRENT_MATRIX_H
#define IZENELIB_AM_CONCURRENT_MATRIX_H

#include <types.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <util/ThreadModel.h>

#include <boost/shared_ptr.hpp>
#include <cassert>
#include <iostream>

NS_IZENELIB_AM_BEGIN

/**
 * @c ConcurrentMatrix is a map from @p KeyType to @p RowType,
 * @p RowType is a map from column key to element,
 * @c RowType::size() is prerequisite to count the total number of entries in the matrix.
 */
template<typename KeyType,
         typename RowType,
         typename ContainerType = ::rde::hash_map<KeyType, boost::shared_ptr<RowType> >,
         typename LockType = izenelib::util::ReadWriteLock>
class ConcurrentMatrix
{
public:
    typedef KeyType key_type;
    typedef RowType row_type;
    typedef ContainerType container_type;
    typedef typename container_type::size_type size_type;

    enum
    {
        ELEM_SIZE = sizeof(typename row_type::key_type) + sizeof(typename row_type::mapped_type),
        ROW_SIZE = sizeof(typename container_type::key_type) + sizeof(typename container_type::mapped_type)
    };

    ConcurrentMatrix() : elemCount_(0) {}

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
        return ELEM_SIZE*elemCount_ + ROW_SIZE*size();
    }

    std::size_t elem_count() const
    {
        return elemCount_;
    }

    void incre_elem_count()
    {
        ScopedWriteLock lock(lock_);
        ++elemCount_;
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
    }

    boost::shared_ptr<RowType> erase(const key_type& key)
    {
        ScopedWriteLock lock(lock_);

        boost::shared_ptr<RowType> row;
        iterator it = container_.find(key);
        if (it != container_.end())
        {
            row = it->second;
            container_.erase(it);

            assert(elemCount_ >= row->size());
            elemCount_ -= row->size();
        }

        return row;
    }

    void clear()
    {
        ScopedWriteLock lock(lock_);
        container_.clear();
        elemCount_ = 0;
    }

    void print(std::ostream& ostream) const
    {
        ostream << "ELEM_SIZE[" << ELEM_SIZE
                << "]*elem[" << elemCount_
                << "] + ROW_SIZE[" << ROW_SIZE
                << "]*row[" << size()
                << "] => ConcurrentMatrix[" << occupy_size() << "]";
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
};

template<typename KeyType, typename RowType, typename ContainerType, typename LockType>
std::ostream& operator<<(
    std::ostream& out,
    const ConcurrentMatrix<KeyType, RowType, ContainerType, LockType>& matrix
)
{
    matrix.print(out);
    return out;
}

NS_IZENELIB_AM_END

#endif // IZENELIB_AM_CONCURRENT_MATRIX_H
