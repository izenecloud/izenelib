#ifndef IZENELIB_AM_MATRIX_DB_H
#define IZENELIB_AM_MATRIX_DB_H

#include <types.h>
#include <am/leveldb/Table.h>
#include <sdb/SequentialDB.h>
#include <sdb/SDBCursorIterator.h>

#include <3rdparty/am/google/sparse_hash_map>
#include <3rdparty/am/stx/btree_map>
#include <3rdparty/am/stx/btree_set>
#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <util/timestamp.h>
#include <util/izene_serialization.h>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <set>
#include <map>
#include <list>
#include <iostream>

NS_IZENELIB_AM_BEGIN

namespace detail{
template <typename KeyType>
class policy_lfu
{
    typedef ::stx::btree_set<KeyType> keyset_type;
    typedef ::stx::btree_map<unsigned long long,keyset_type > entry_type;
    typedef ::rde::hash_map<KeyType, unsigned long long> backentry_type;
    entry_type _entries;
    backentry_type _backEntries;
public:
    void insert(const KeyType& _k)
    {
        keyset_type& pad=_entries[1];
        pad.insert(_k);
        _backEntries.insert(rde::pair<KeyType,unsigned long long>(_k,1));
    }

    void remove(const KeyType& _k)
    {
        unsigned long long ref = _backEntries[_k];
        if(ref > 0 )
        {
            keyset_type& pad = _entries[_backEntries[_k]];
            pad.erase(_k);
            if(pad.empty())
            {
                _entries.erase(_backEntries[_k]);
            }
            _backEntries.erase(_k);
        }
    }

    void touch(const KeyType& _k)
    {
        unsigned long long& ref = _backEntries[_k];

        if(ref > 0)
        {
            keyset_type& pad = _entries[ref];
            pad.erase(_k);
            if(pad.empty())
            {
                _entries.erase(ref);
            }
/*
            typename entry_type::iterator eit = _entries.find(ref);
            if (eit != _entries.end())
            {
                eit->second.erase(_k);
                if (eit->second.empty())
                {
                    _entries.erase(eit);
                }
            }
*/
        }
        ref++;
        keyset_type& new_pad =  _entries[ref];
        new_pad.insert(_k);

    }

    void clear()
    {
        _entries.clear();
        _backEntries.clear();
    }

    size_t size()
    {
        size_t count = 0;
        for(typename entry_type::iterator it = _entries.begin(); it != _entries.end(); ++it)
        {
            count += sizeof(KeyType)*it.data().size();
            count += sizeof(unsigned long long);
        }
        for(typename backentry_type::iterator it = _backEntries.begin(); it != _backEntries.end(); ++it)
        {
            count += sizeof(KeyType);
            count += sizeof(unsigned long long);
        }
        return count;
    }

    bool evict(KeyType& k)
    {
        for(typename entry_type::iterator iter = _entries.begin(); iter != _entries.end(); ++iter)
        {
            keyset_type& pad = iter.data();
            k = *(pad.begin());
            pad.erase(k);
            if(pad.empty())
                _entries.erase(iter.key());
            _backEntries.erase(k);
            return true;

/*			
            typename keyset_type::iterator kit = iter->second.begin();
            if(kit != iter->second.end())
            {
                k = *kit;
                iter->second.erase(kit);
                if (iter->second.empty())
                {
                    _entries.erase(iter);
                }
                _backEntries.erase(k);
                return true;    
            }
*/
        }
        return false;
    }
};

template <class KeyType> 
class policy_lru 
{
    std::list<KeyType> _entries;
public:
    void insert(const KeyType& _k) 
    {
        _entries.push_front(_k);
    }

    void remove(const KeyType& _k)
    {
       _entries.remove(_k);
    }

    void touch(const KeyType& _k)
    { 
        _entries.remove(_k);
        _entries.push_front(_k);
    }
    void clear()
    {
        _entries.clear();
    }

    size_t size()
    {
        return _entries.size() * sizeof(KeyType);
    }

    bool evict(KeyType& k)
    {
        if(_entries.empty()) return false;
        k = _entries.back();
        _entries.remove(k);
        return true;
    }
};

}

template<
typename KeyType,
typename ElementType,
typename RowType = ::google::sparse_hash_map<KeyType, ElementType >,
typename StorageType = izenelib::sdb::unordered_sdb_tc<KeyType, RowType, ReadWriteLock >,
typename IteratorType = izenelib::sdb::SDBCursorIterator<StorageType>,
typename Policy = detail::policy_lfu<KeyType> 
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
        dump();
        _db_storage.flush();
        _db_storage.close();
    }

    iterator begin()
    {
        // before iterating db on disk,
        // the dirty caches in memeory need to be flushed to disk
        dump();

        return iterator(_db_storage);
    }

    iterator end()
    {
        return iterator();
    }

    bool erase ( const KeyType& x )
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

    void coeff(KeyType x, KeyType y, ElementType d)
    {
        boost::shared_ptr<RowType> row_data = _row(x);
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

    void incre(KeyType x, KeyType y, ElementType inc)
    {
        boost::shared_ptr<RowType> row_data = _row(x);
        _cache_row_dirty_flag.insert(x);
        typename RowType::iterator it = row_data->find(y); 
        if(it == row_data->end())
        {
            (*row_data)[y] = inc;
            _currEntries++;
        }
        else
        {
            it->second += inc;
        }
    }

    void incre(KeyType x, KeyType y)
    {
        boost::shared_ptr<RowType> row_data = _row(x);
        _cache_row_dirty_flag.insert(x);
        typename RowType::iterator it = row_data->find(y); 
        if(it == row_data->end())
        {
            ElementType& e = (*row_data)[y];
            e.update();
            _currEntries++;
        }
        else
        {
            it->second.update();;
        }
    }

    void row_incre(KeyType x, const std::list<KeyType>& cols)
    {
        boost::shared_ptr<RowType> row_data = _row(x);
        _cache_row_dirty_flag.insert(x);
        for(typename std::list<KeyType>::const_iterator iter = cols.begin();
            iter != cols.end(); ++iter)
        {
            typename RowType::iterator it = row_data->find(*iter); 
            if(it == row_data->end())
            {
                ElementType& e = (*row_data)[*iter];
                e.update();
                _currEntries++;
            }
            else
            {
                it->second.update();
            }
        }
    }

    ElementType coeff(KeyType x, KeyType y)
    {
        boost::shared_ptr<RowType> row_data = _row(x);
        typename RowType::iterator it = row_data->find(y); 
        if(it == row_data->end()) return ElementType();
        return it->second;
    }

    boost::shared_ptr<const RowType> row(KeyType x)
    {
        return _row(x);
    }

    /**
     * update @p row_data to db storage,
     * if there is old data in cache, replace it with @p row_data.
     * @param x row number
     * @param row_data new row data to update
     */
    void update_row_without_cache(KeyType x, const RowType& row_data)
    {
        typename CacheStorageType::iterator cit = _cache_storage.find(x);
        if(cit != _cache_storage.end())
        {
            // update entry count
            boost::shared_ptr<RowType > old_row_data(cit->second);
            size_t old_row_size = old_row_data->size();
            _currEntries = (_currEntries <= old_row_size)?0:(_currEntries - old_row_size);
            _currEntries += row_data.size();

            // replace cache
            cit->second.reset(new RowType(row_data));

            // remove dirty flag
            typename CacheDirtyFlagSet::iterator fit = _cache_row_dirty_flag.find(x);
            if (fit != _cache_row_dirty_flag.end())
            {
                _cache_row_dirty_flag.erase(fit);
            }
        }

        _db_storage.update(x, row_data);
    }

    bool row_without_cache(KeyType x, RowType& row_data)
    {
        return _db_storage.get(x,row_data);
    }

    void clear()
    {
        dump();
        _cache_storage.clear();
    }

    void dump()
    {
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
    void _evict()
    {
        while (this->_currEntries >= this->_maxEntries)
        {
            KeyType evict;
            if(_policy.evict(evict))
                erase(evict);
            else
                break;
        }
    }

    boost::shared_ptr<RowType> _row(KeyType x)
    {
        _evict();

        boost::shared_ptr<RowType > row_data;
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
            row_data = cit->second;

        _policy.touch(x);

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

