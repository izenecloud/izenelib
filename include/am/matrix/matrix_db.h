#ifndef IZENELIB_AM_MATRIX_DB_H
#define IZENELIB_AM_MATRIX_DB_H

#include <types.h>
#include "matrix_cache.h"
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
    typedef MatrixCache<KeyType, RowType> CacheType;
    CacheType _cache;

    const std::size_t _max_cache_elem;
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

    void update_elem(KeyType x, KeyType y, ElementType d)
    {
        if (_cache.update_elem(x, y, d))
        {
            _policy.touch(x);
            return;
        }

        boost::shared_ptr<RowType> row(new RowType);
        _db_storage.get(x, *row);
        (*row)[y] = d;
        update_row(x, row);
    }

    boost::shared_ptr<const RowType> row(KeyType x)
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

    void update_row(KeyType x, boost::shared_ptr<RowType> row)
    {
        _evict();
        _cache.update(x, row);
        _policy.touch(x);
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
        size_t policySize = _policy.size();
        size_t totalSize = cacheSize + policySize;

        ostream << _cache
                << " + policy[" << policySize
                << "] => MatrixDB[" << totalSize << "]";
        if (_is_cache_full())
        {
            ostream << " FULL";
        }
    }

private:
    void _evict()
    {
        KeyType key;
        while (_is_cache_full() && _policy.evict(key))
        {
            bool isDirty = false;
            boost::shared_ptr<RowType> row = _cache.erase(key, isDirty);
            if (row && isDirty)
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
        KeyType key;
        boost::shared_ptr<RowType> row;
        while ((row = _cache.reset_dirty_flag(key)))
        {
            _db_storage.update(key, *row);
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
