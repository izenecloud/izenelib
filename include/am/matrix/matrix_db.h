#ifndef IZENELIB_AM_MATRIX_DB_H
#define IZENELIB_AM_MATRIX_DB_H

#include <types.h>
#include "concurrent_set.h"
#include "concurrent_matrix.h"
#include <am/detail/cache_policy.h>

#include <sdb/SequentialDB.h>
#include <sdb/SDBCursorIterator.h>

#include <3rdparty/am/google/sparse_hash_map>
#include <util/izene_serialization.h>

#include <boost/shared_ptr.hpp>
#include <iostream>

NS_IZENELIB_AM_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
//MatrixDB: Persistent matrix data with two features:
// 1. Parametering cache size according to all matrix elements, instead of row numbers
// 2. Write-back (or Write-behind) cache instead of Write through, which means writing is done 
//     only to the cache. A modified cache block is written back to the store, just before it is replaced.
//
////////////////////////////////////////////////////////////////////////////////
template<
typename KeyType,
typename ElementType,
typename RowType = ::google::sparse_hash_map<KeyType, ElementType >,
typename StorageType = izenelib::sdb::unordered_sdb_tc<KeyType, RowType, ReadWriteLock >,
typename IteratorType = izenelib::sdb::SDBCursorIterator<StorageType>,
typename Policy = detail::policy_lfu_nouveau<KeyType> 
>
class MatrixDB
{
    typedef ConcurrentMatrix<KeyType, RowType> CacheType;
    CacheType _cache;
    const std::size_t _max_cache_elem;

    typedef ConcurrentSet<KeyType> DirtyFlags;
    DirtyFlags _dirty_flags;

    StorageType _db_storage;
    Policy _policy;

public:
    typedef RowType row_type;
    typedef IteratorType iterator; // first is KeyType, second is RowType

    explicit MatrixDB(size_t size, const std::string& path)
        : _max_cache_elem(size/CacheType::ELEM_SIZE)
        , _db_storage(path)
    {
        _db_storage.open();
    }

    ~MatrixDB()
    {
        flush();
        _db_storage.close();
    }

    void flush()
    {
        _dump();
        _db_storage.flush();
    }

    iterator begin()
    {
        // before iterating db on disk,
        // the dirty caches in memeory need to be flushed to disk
        _dump();

        return iterator(_db_storage);
    }

    iterator end()
    {
        return iterator();
    }

    ElementType elem(KeyType x, KeyType y)
    {
        boost::shared_ptr<const RowType> row_data = row(x);
        typename RowType::const_iterator it = row_data->find(y); 

        if(it == row_data->end())
            return ElementType();

        return it->second;
    }

    /**
     * @attention as @c RowType is not thread-safe, below things are forbidden:
     * 1. if you call this function with the same @p x in multi-threads, the result in updating row @p x is undefined.
     * 2. if you call this function in one thread, and iterate @c RowType of the same @p x in another thread, the iteration result is undefined.
     */
    void update_elem(KeyType x, KeyType y, ElementType d)
    {
        boost::shared_ptr<RowType> row = _row_impl(x);
        typename RowType::iterator it = row->find(y);
        if(it == row->end())
        {
            (*row)[y] = d;
            _cache.incre_elem_count();
        }
        else
        {
            it->second = d;
        }
        _dirty_flags.insert(x);
    }

    boost::shared_ptr<const RowType> row(KeyType x)
    {
        return _row_impl(x);
    }

    void update_row(KeyType x, boost::shared_ptr<RowType> row)
    {
        _evict();
        _cache.update(x, row);
        _policy.touch(x);
        _dirty_flags.insert(x);
    }

    void clear()
    {
        _dump();
        _cache.clear();
        _policy.clear();
    }

    void print(std::ostream& ostream) const
    {
        size_t cacheSize = _cache.occupy_size();
        size_t flagSize = _dirty_flags.occupy_size();
        size_t policySize = _policy.size();
        size_t totalSize = cacheSize + flagSize + policySize;

        ostream << _cache
                << " + flag[" << flagSize
                << "] + policy[" << policySize
                << "] => MatrixDB[" << totalSize << "]";
        if (_is_cache_full())
        {
            ostream << " FULL";
        }
    }

private:
    boost::shared_ptr<RowType> _row_impl(KeyType x)
    {
        boost::shared_ptr<RowType> row = _cache.get(x);
        if (! row)
        {
            row.reset(new RowType);
            _db_storage.get(x, *row);

            _evict();
            _cache.insert(x, row);
        }
        _policy.touch(x);

        return row;
    }

    void _evict()
    {
        KeyType key;
        while (_is_cache_full() && _policy.evict(key))
        {
            boost::shared_ptr<RowType> row = _cache.erase(key);
            if (row && _dirty_flags.erase(key))
            {
                _db_storage.update(key, *row);
            }
        }
    }

    bool _is_cache_full() const
    {
        return _cache.elem_count() >= _max_cache_elem;
    }

    void _dump()
    {
        typedef typename DirtyFlags::container_type FlagContainer;
        FlagContainer flags;
        _dirty_flags.swap(flags);

        for (typename FlagContainer::const_iterator fit = flags.begin();
            fit != flags.end(); ++fit)
        {
            boost::shared_ptr<RowType> row = _cache.get(*fit);
            if (row)
            {
                _db_storage.update(*fit, *(row));
            }
        }
    }
};

template<typename KeyType, typename ElementType, typename RowType, typename StorageType, typename IteratorType, typename Policy>
std::ostream& operator<<(
    std::ostream& out,
    const MatrixDB<KeyType, ElementType, RowType, StorageType, IteratorType, Policy>& matrix
)
{
    matrix.print(out);
    return out;
}

NS_IZENELIB_AM_END

namespace izenelib{namespace util{

struct pod_tag{};

template <typename T1,typename T2>
struct IsFebirdSerial< ::google::sparse_hash_map<T1, T2 >  >{
    //enum {yes = IsFebirdSerial<T1 >::yes && IsFebirdSerial<T2 >::yes, no= !yes};
    //for compatibility issue, is_pod is not used within the definition of IsFebirdSerial
    enum {yes =( boost::is_pod<T1>::value || boost::is_base_of<pod_tag, T1>::value ) 
                  && ( boost::is_pod<T2>::value || boost::is_base_of<pod_tag, T2>::value) , no= !yes};
};

}}

#endif

