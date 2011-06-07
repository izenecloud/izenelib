#ifndef IZENELIB_AM_MATRIX_DB_H
#define IZENELIB_AM_MATRIX_DB_H

#include <types.h>
#include <am/leveldb/Table.h>
#include <am/3rdparty/rde_hash.h>
#include <sdb/SequentialDB.h>
#include <3rdparty/am/google/sparse_hash_map>
#include <3rdparty/am/stx/btree_map>
#include <3rdparty/am/stx/btree_set>

#include <util/timestamp.h>
#include <util/izene_serialization.h>

#include <boost/shared_ptr.hpp>

#include <set>
#include <iostream>

NS_IZENELIB_AM_BEGIN

namespace detail{
template <typename KeyType>
class policy_lfu
{
    typedef ::stx::btree_set<KeyType> keySet;
    ::stx::btree_map<unsigned long long,keySet > _entries;
    ::stx::btree_map<KeyType, unsigned long long> _backEntries;
public:
    policy_lfu() {}

    void insert(const KeyType& _k)
    {
        keySet pad=_entries[1];
        pad.insert(_k);
        if (!_backEntries.insert(std::pair<KeyType,unsigned long long>(_k,1)).second)
        {
            return;
        }
        _entries.erase(1);
        _entries.insert(std::pair<unsigned long long, keySet>(1,pad));
    }

    void remove(const KeyType& _k)
    {
        keySet pad = _entries[_backEntries[_k]];

        pad.erase(_k);
        _entries.erase(_backEntries[_k]);
        if (pad.size()>0)
        {
            _entries.insert(std::pair<unsigned long long, keySet>(_backEntries[_k],pad));
        }

        _backEntries.erase(_k);
    }

    void touch(const KeyType& _k)
    {
        unsigned long long ref = _backEntries[_k];

        keySet pad = _entries[ref];
        pad.erase(_k);
        _entries.erase(ref);
        if (!pad.empty())
        {
            _entries.insert(std::pair<unsigned long long, keySet>(ref,pad));
        }

        ref++;
        if (_entries.find(ref)!=_entries.end())
        {
            pad = _entries[ref];
        }
        else
        {
            pad = keySet();
        }
        pad.insert(_k);
        _entries.erase(ref);
        _entries.insert(std::pair<unsigned long long, keySet>(ref,pad));

        _backEntries.erase(_k);
        _backEntries[_k]=ref;
    }
    void clear()
    {
        _entries.clear();
        _backEntries.clear();
    }

    bool evict(KeyType& k)
    {
        if (_entries.begin()==_entries.end())
        {
            return false;
        }
        keySet pad=(*(_entries.begin())).second; //Begin returns entry with lowest id, just what we need:)

        if (pad.begin()==pad.end())
        {
            return false;
        }
        k = *(pad.begin());
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
    typedef izenelib::am::rde_hash<KeyType, boost::shared_ptr<RowType > > CacheStorageType;
    CacheStorageType _cache_storage;
    typedef ::rde::hash_map<KeyType, bool > CacheFlagType;
    CacheFlagType _cache_flag;
    enum { ElementSize = sizeof(KeyType)+sizeof(ElementType) };
    std::size_t _maxEntries;
    std::size_t _currEntries;
    StorageType _db_storage;
    Policy _policy;
public:
    explicit MatrixDB(size_t size, const std::string& path)
        :_maxEntries(size/ElementSize), _currEntries(0), _db_storage(path)
    {
        _db_storage.open();
    }

    ~MatrixDB()
    {
        dump();
        _db_storage.close();
    }

    void erase ( const KeyType& x )
    {
        boost::shared_ptr<RowType > row_data;
        if(_cache_storage.get(x,row_data))
        {
            _currEntries -= (row_data->size());
            _db_storage.update(x, *row_data);
            _cache_storage.del(x);
            _policy.remove(x);
            _set_flag(x,false);
        }
    }

    void coeff(KeyType x, KeyType y, ElementType d)
    {
        boost::shared_ptr<RowType> row_data;
        row(x, row_data);
        _set_flag(x,true);
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
        boost::shared_ptr<RowType> row_data;
        row(x, row_data);
        _set_flag(x,true);
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
        boost::shared_ptr<RowType> row_data;
        row(x, row_data);
        _set_flag(x,true);
        ElementType& e = (*row_data)[y];
        e.update();
    }

    ElementType coeff(KeyType x, KeyType y)
    {
        boost::shared_ptr<RowType> row_data;
        row(x, row_data);
        typename RowType::iterator it = row_data->find(y); 
        if(it == row_data->end()) return ElementType();
        return it->second;
    }

    bool row(KeyType x, boost::shared_ptr<RowType >& row_data)
    {
        _touch(x);
        if(!_cache_storage.get(x, row_data))
        {
            row_data.reset(new RowType);
            _db_storage.get(x,*row_data);
            _cache_storage.insert(x,row_data);
            return false;
        }
        else return true;
    }

    void dump()
    {
        typename CacheFlagType::iterator it = _cache_flag.begin();
        for(; it != _cache_flag.end(); ++it)
        {
            if(it->second)
            {
                boost::shared_ptr<RowType > row_data;
                if(_cache_storage.get(it->first,row_data))
                {
                    _db_storage.update(it->first, *row_data);
                    it->second = false;
                }
            }
        }
    }

private:
    void _touch(const KeyType& x)
    {
        while (this->_currEntries >= this->_maxEntries)
        {
            KeyType evict;
            _policy.evict(evict);
            this->erase(evict);
        }
        _policy.insert(x);
    }


    void _set_flag(const KeyType& x, bool flag)
    {
        typename CacheFlagType::iterator it = _cache_flag.find(x);
        if(it == _cache_flag.end())
        {
            _cache_flag.insert( rde::pair<KeyType, bool>(x, flag) );
        }
        else
            it->second = flag;
    }

};


NS_IZENELIB_AM_END

namespace izenelib{namespace util{

template <typename T1,typename T2>
struct IsFebirdSerial< ::google::sparse_hash_map<T1, T2 >  >{
    //enum {yes = IsFebirdSerial<T1 >::yes && IsFebirdSerial<T2 >::yes, no= !yes};
    //for compatibility issue, is_pod is not used within the definition of IsFebirdSerial
    enum {yes = is_pod<T1>::value && is_pod<T2>::value, no= !yes};
};

}}

#endif

