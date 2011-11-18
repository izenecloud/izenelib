#ifndef IZENELIB_AM_MATRIX_DB_H
#define IZENELIB_AM_MATRIX_DB_H

#include <types.h>
#include <am/leveldb/Table.h>
#include <am/detail/cache_policy.h>

#include <sdb/SequentialDB.h>
#include <sdb/SDBCursorIterator.h>

#include <3rdparty/am/google/sparse_hash_map>
 
#include <util/timestamp.h>
#include <util/izene_serialization.h>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>

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
    typedef ::rde::hash_map<KeyType, boost::shared_ptr<RowType > > CacheStorageType;
    CacheStorageType _cache_storage;
    typedef ::stx::btree_set<KeyType> CacheDirtyFlagSet;
    CacheDirtyFlagSet _cache_row_dirty_flag;
    enum { ElementSize = sizeof(KeyType)+sizeof(ElementType) };
    std::size_t _maxEntries;
    std::size_t _currEntries;
    StorageType _db_storage;
    Policy _policy;
    boost::mutex _mutex;

public:
    typedef RowType row_type;
    typedef IteratorType iterator; // first is KeyType, second is RowType

    explicit MatrixDB(size_t size, const std::string& path)
        :_maxEntries(size/ElementSize), _currEntries(0), _db_storage(path)
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

    bool erase(const KeyType& x)
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _erase_impl(x);
    }

    void coeff(KeyType x, KeyType y, ElementType d)
    {
        boost::mutex::scoped_lock lock(_mutex);

        boost::shared_ptr<RowType> row_data = _row_impl(x);
        _cache_row_dirty_flag.insert(x);
        typename RowType::iterator it = row_data->find(y); 
        if(it == row_data->end())
        {
            (*row_data)[y] = d;
            _currEntries++;
        }
        else
        {
            it->second = d;
        }
    }

    ElementType coeff(KeyType x, KeyType y)
    {
        boost::shared_ptr<const RowType> row_data = row(x);
        typename RowType::const_iterator it = row_data->find(y); 
        if(it == row_data->end())
            return ElementType();
        return it->second;
    }

    boost::shared_ptr<const RowType> row(KeyType x)
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _row_impl(x);
    }

    /**
     * update row @p x with @p row_data.
     * @param x row number
     * @param row_data new row data to update
     * @post for performance consideration, after this function is called, @p row_data
     * would become empty as its contents is swapped with a new @c RowType.
     */
    void update_row(KeyType x, RowType& row_data)
    {
        boost::mutex::scoped_lock lock(_mutex);

        boost::shared_ptr<RowType> new_row_data(new RowType);
        new_row_data->swap(row_data);

        typename CacheStorageType::iterator cit = _cache_storage.find(x);
        if(cit != _cache_storage.end())
        {
            // remove old entry count
            boost::shared_ptr<RowType> old_row_data(cit->second);
            size_t old_row_size = old_row_data->size();
            _currEntries = (_currEntries <= old_row_size)?0:(_currEntries - old_row_size);

            // add new entry count
            _evict();
            _currEntries += new_row_data->size();

            // replace cache
            cit->second = new_row_data;
        }
        else
        {
            // add new entry count
            _evict();
            _currEntries += new_row_data->size();

            // insert cache
            _cache_storage.insert(rde::make_pair(x,new_row_data));
        }

        _policy.touch(x);
        _cache_row_dirty_flag.insert(x);
    }

    bool row_without_cache(KeyType x, RowType& row_data)
    {
        return _db_storage.get(x,row_data);
    }

    void clear()
    {
        _dump();

        boost::mutex::scoped_lock lock(_mutex);
        _cache_storage.clear();
    }

    void status(std::ostream& ostream)
    {
        size_t poolcount = 0;
        typename CacheStorageType::iterator cit = _cache_storage.begin();
        size_t i = 0;
        for(;cit != _cache_storage.end() ; ++cit)
        {
            poolcount += (cit->second->size())*ElementSize;
            ++i;
        }
        size_t policycount = _policy.size();
        size_t flagcount = (sizeof(KeyType))*_cache_row_dirty_flag.size();
        size_t cachecount = (sizeof(KeyType)+sizeof(void*))*_cache_storage.size();
        size_t count = policycount + flagcount + cachecount + poolcount;
        ostream<<"_cache_storage size "<<_cache_storage.size()<<" poolcount "<<poolcount<<" policycount "<<policycount<<" flagcount "<<flagcount<<" cachecount "<<cachecount<<" count "<<count<<std::endl;
    }

private:
    void _dump()
    {
        boost::mutex::scoped_lock lock(_mutex);

        typename CacheDirtyFlagSet::iterator it = _cache_row_dirty_flag.begin();
        for(; it != _cache_row_dirty_flag.end(); ++it)
        {
            typename CacheStorageType::iterator cit = _cache_storage.find(*it);
            if(cit != _cache_storage.end())
            {
                _db_storage.update(*it, *(cit->second));
            }
        }
        _cache_row_dirty_flag.clear();
    }

    bool _erase_impl(const KeyType& x)
    {
        typename CacheStorageType::iterator cit = _cache_storage.find(x);
        if(cit != _cache_storage.end())
        {
            boost::shared_ptr<RowType > row_data(cit->second);
            size_t row_size = row_data->size();
            _currEntries = (_currEntries <= row_size)?0:(_currEntries - row_size);
            typename CacheDirtyFlagSet::iterator fit = _cache_row_dirty_flag.find(x);
            if (fit != _cache_row_dirty_flag.end())
            {
                _db_storage.update(x, *row_data);
                _cache_row_dirty_flag.erase(fit);
            }
            _cache_storage.erase(cit);
            return true;
        }
        return false;
    }

    void _evict()
    {
        while (this->_currEntries >= this->_maxEntries)
        {
            KeyType evict;
            if(_policy.evict(evict))
                _erase_impl(evict);
            else
                break;
        }
    }

    boost::shared_ptr<RowType> _row_impl(KeyType x)
    {
        _evict();
        _policy.touch(x);

        boost::shared_ptr<RowType> row_data;
        typename CacheStorageType::iterator cit = _cache_storage.find(x);		
        if(cit == _cache_storage.end())
        {
            row_data.reset(new RowType);
            if(_db_storage.get(x,*row_data))
            {
                _currEntries+=row_data->size();
            }
            _cache_storage.insert(rde::make_pair(x,row_data));
        }
        else
        {
            row_data = cit->second;
        }

        return row_data;
    }

};

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

