#ifndef IZENELIB_AM_MATRIX_DB_H
#define IZENELIB_AM_MATRIX_DB_H

#include <types.h>
#include <am/leveldb/Table.h>
#include <sdb/SequentialDB.h>

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
    typedef ::stx::btree_map<KeyType, unsigned long long> backentry_type;
    entry_type _entries;
    backentry_type _backEntries;
public:
    void insert(const KeyType& _k)
    {
        keyset_type& pad=_entries[1];
        pad.insert(_k);
        _backEntries.insert(std::pair<KeyType,unsigned long long>(_k,1));
    }

    void remove(const KeyType& _k)
    {
        keyset_type& pad = _entries[_backEntries[_k]];
        pad.erase(_k);
        _backEntries.erase(_k);
    }

    void touch(const KeyType& _k)
    {
        unsigned long long& ref = _backEntries[_k];

        if(ref > 0)
        {
            keyset_type& pad = _entries[ref];
            pad.erase(_k);
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
            count += sizeof(KeyType)*it->second.size();
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
            if(! iter->second.empty())
            {
                k = *(iter->second.begin());
                remove(k);
                return true;				
            }
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
typename StorageType =izenelib::sdb::unordered_sdb_tc<KeyType, RowType, ReadWriteLock >,
typename Policy = detail::policy_lfu<KeyType> 
>
class MatrixDB
{
    typedef ::rde::hash_map<KeyType, boost::shared_ptr<RowType > > CacheStorageType;
    CacheStorageType _cache_storage;
    typedef ::rde::hash_map<KeyType, bool > CacheDirtyFlagType;
    CacheDirtyFlagType _cache_row_dirty_flag;
    enum { ElementSize = sizeof(KeyType)+sizeof(ElementType) };
    std::size_t _maxEntries;
    std::size_t _currEntries;
    StorageType _db_storage;
    Policy _policy;
public:
    typedef RowType row_type;
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

    bool erase ( const KeyType& x )
    {
        boost::shared_ptr<RowType > row_data;
        typename CacheStorageType::iterator cit = _cache_storage.find(x);
        if(cit != _cache_storage.end())
        {
            row_data = cit->second;
            size_t row_size = row_data->size();
            _currEntries = (_currEntries <= row_size)?0:(_currEntries - row_size);
            _db_storage.update(x, *row_data);
            _cache_storage.erase(x);
            _set_row_dirty_flag(x,false);
            return true;
        }
        return false;
    }

    void coeff(KeyType x, KeyType y, ElementType d)
    {
        boost::shared_ptr<RowType> row_data = row(x);
        _set_row_dirty_flag(x,true);		
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
        boost::shared_ptr<RowType> row_data = row(x);
        _set_row_dirty_flag(x,true);
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
        boost::shared_ptr<RowType> row_data = row(x);
        _set_row_dirty_flag(x,true);
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
        boost::shared_ptr<RowType> row_data = row(x);
        _set_row_dirty_flag(x,true);		
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
        boost::shared_ptr<RowType> row_data = row(x);
        typename RowType::iterator it = row_data->find(y); 
        if(it == row_data->end()) return ElementType();
        return it->second;
    }

    boost::shared_ptr<RowType > row(KeyType x)
    {
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
            _set_row_dirty_flag(x,false);
        }
        else
            row_data = cit->second;

        _touch(x);
        return row_data;
    }

    void dump()
    {
        typename CacheDirtyFlagType::iterator it = _cache_row_dirty_flag.begin();
        for(; it != _cache_row_dirty_flag.end(); ++it)
        {
            if(it->second)
            {
                typename CacheStorageType::iterator cit = _cache_storage.find(it->first);
                if(cit != _cache_storage.end())
                {
                    _db_storage.update(it->first, *(cit->second));
                    it->second = false;
                }
            }
        }
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
        size_t flagcount = (sizeof(KeyType)+sizeof(bool))*_cache_row_dirty_flag.size();
        size_t cachecount = (sizeof(KeyType)+sizeof(void*))*_cache_storage.size();
        size_t count = policycount + flagcount + cachecount + poolcount;
        ostream<<"_cache_storage size "<<_cache_storage.size()<<" poolcount "<<poolcount<<" policycount "<<policycount<<" flagcount "<<flagcount<<" cachecount "<<cachecount<<" count "<<count<<std::endl;
    }

private:
    void _touch(const KeyType& x)
    {
        _policy.touch(x);
        while (this->_currEntries >= this->_maxEntries)
        {
            KeyType evict;
            if(_policy.evict(evict))
                erase(evict);
            else
                break;
        }
    }

    void _set_row_dirty_flag(const KeyType& x, bool flag)
    {
        typename CacheDirtyFlagType::iterator it = _cache_row_dirty_flag.find(x);
        if(it == _cache_row_dirty_flag.end())
        {
            _cache_row_dirty_flag.insert( rde::pair<KeyType, bool>(x, flag) );
        }
        else
            it->second = flag;
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

