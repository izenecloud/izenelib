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
class policy_lfu_nouveau
{
    struct cache_entry;
    struct freq_node
    {
        unsigned long _freq, _count;
        struct cache_entry *_newest, *_oldest;
        struct freq_node *_hotter, *_colder;

        freq_node(unsigned long freq = 1)
        {
            _freq = freq;
            _count = 0;
            _newest = _oldest = NULL;
            _hotter = _colder = NULL;
        }

        void insert(struct cache_entry *new_entry)
        {
            new_entry->_newer = NULL;
            new_entry->_older = _newest;
            if (!_oldest)
                _oldest = new_entry;
            if (_newest)
                _newest->_newer = new_entry;
            _newest = new_entry;
            _count++;
        }

        bool evict(KeyType& k)
        {
            k = _oldest->_key;
            struct cache_entry *oldest = _oldest->_newer;
            delete _oldest;
            _oldest = oldest;
            _count--;

            if (_oldest)
            {
                _oldest->_older = NULL;
                return true;
            }
            else
                return false;
        }

        void clear()
        {
            while (_oldest)
            {
                struct cache_entry *next = _oldest->_newer;
                delete _oldest;
                _oldest = next;
            }
        }
    };

    struct cache_entry
    {
        struct cache_entry *_newer, *_older;
        struct freq_node *_parent;
        KeyType _key;

        cache_entry(const KeyType& k)
        {
            _newer = _older = NULL;
            _parent = NULL;
            _key = k;
        }
    };

    typedef ::rde::hash_map<KeyType, struct cache_entry *> backentry_type;
    struct freq_node *_freq_list_head;
    backentry_type _backEntries;

public:
    void insert(const KeyType& k)
    {
        struct cache_entry *new_entry = new cache_entry(k);
        if (!_freq_list_head || _freq_list_head->_freq != 1)
        {
            struct freq_node *new_freq_node = new freq_node();
            new_freq_node->_hotter = _freq_list_head;
            if (_freq_list_head)
                _freq_list_head->_colder = new_freq_node;
            _freq_list_head = new_freq_node;
        }
        _freq_list_head->insert(new_entry);
        new_entry->_parent = _freq_list_head;
        _backEntries.insert(rde::pair<KeyType, struct cache_entry *>(k, new_entry));
    }

    void remove(const KeyType& k)
    {
        struct cache_entry *ref = _backEntries[k];
        if (ref)
        {
            struct freq_node *parent = ref->_parent;
            if (ref->_newer)
                ref->_newer->_older = ref->_older;
            else
                parent->_newest = ref->_older;

            if (ref->_older)
                ref->_older->_newer = ref->_newer;
            else
                parent->_oldest = ref->_newer;

            delete ref;
            if (!parent->_newest)
            {
                if (parent->_hotter)
                    parent->_hotter->_colder = parent->_colder;

                if (parent->_colder)
                    parent->_colder->_hotter = parent->_hotter;
                else
                    _freq_list_head = parent->_hotter;

                delete parent;
            }
            else
                parent->_count--;
            _backEntries.erase(k);
        }
    }

    void touch(const KeyType& k)
    {
        struct cache_entry *ref = _backEntries[k];
        if (ref)
        {
            struct freq_node *parent = ref->_parent;
            if (ref->_newer)
                ref->_newer->_older = ref->_older;
            else
                parent->_newest = ref->_older;

            if (ref->_older)
                ref->_older->_newer = ref->_newer;
            else
                parent->_oldest = ref->_newer;

            struct freq_node *hotter = parent->_hotter;
            if (!hotter || (hotter->_freq != parent->_freq + 1))
            {
                struct freq_node *hotter = new freq_node(parent->_freq + 1);
                hotter->_colder = parent;
                hotter->_hotter = parent->_hotter;
                if (parent->_hotter)
                    parent->_hotter->_colder = hotter;
            }
            hotter->insert(ref);
            ref->_parent = hotter;
            if (!parent->_newest)
            {
                if (parent->_hotter)
                    parent->_hotter->_colder = parent->_colder;

                if (parent->_colder)
                    parent->_colder->_hotter = parent->_hotter;
                else
                    _freq_list_head = parent->_hotter;

                delete parent;
            }
            else
                parent->_count--;
        }
        else
            insert(k);
    }

    void clear()
    {
        while (_freq_list_head)
        {
            _freq_list_head->clear();
            struct freq_node *freq_list_head = _freq_list_head->hotter;
            delete _freq_list_head;
            _freq_list_head = freq_list_head;
        }
        _backEntries.clear();
    }

    size_t size()
    {
        size_t count = 0;
        for (struct freq_node *it = _freq_list_head; it != NULL; it = it->_hotter)
        {
            count += sizeof(struct cache_entry) * it->_count;
            count += sizeof(struct freq_node);
        }
        count += _backEntries.size() * (sizeof(KeyType) * sizeof(struct cache_entry *));
        return count;
    }

    bool evict(KeyType& k)
    {
        if (!_freq_list_head)
            return false;

        if (!_freq_list_head->evict(k))
        {
            struct freq_node *freq_list_head = _freq_list_head->_hotter;
            delete _freq_list_head;
            _freq_list_head = freq_list_head;
            if (_freq_list_head)
                _freq_list_head->_colder = NULL;
        }
        _backEntries.erase(k);

        return true;
    }
};

template <typename KeyType>
class policy_lfu
{
    typedef ::stx::btree_set<KeyType> keyset_type;
    typedef ::stx::btree_map<unsigned long, keyset_type > entry_type;
    typedef ::rde::hash_map<KeyType, unsigned long> backentry_type;
    entry_type _entries;
    backentry_type _backEntries;

public:
    void insert(const KeyType& k)
    {
        keyset_type& pad=_entries[1];
        pad.insert(k);
        _backEntries.insert(rde::pair<KeyType, unsigned long long>(k, 1));
    }

    void remove(const KeyType& k)
    {
        unsigned long ref = _backEntries[k];
        if(ref > 0 )
        {
            keyset_type& pad = _entries[ref];
            pad.erase(k);
            if(pad.empty())
            {
                _entries.erase(ref);
            }
            _backEntries.erase(k);
        }
    }

    void touch(const KeyType& k)
    {
        unsigned long& ref = _backEntries[k];

        if(ref > 0)
        {
            keyset_type& pad = _entries[ref];
            pad.erase(k);
            if(pad.empty())
            {
                _entries.erase(ref);
            }
        }
        ref++;
        keyset_type& new_pad =  _entries[ref];
        new_pad.insert(k);

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
            count += sizeof(unsigned long);
        }
        for(typename backentry_type::iterator it = _backEntries.begin(); it != _backEntries.end(); ++it)
        {
            count += sizeof(KeyType);
            count += sizeof(unsigned long);
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
                _entries.erase(iter);
            _backEntries.erase(k);
            return true;
        }
        return false;
    }
};

template <class KeyType> 
class policy_lru 
{
    std::list<KeyType> _entries;

public:
    void insert(const KeyType& k) 
    {
        _entries.push_front(k);
    }

    void remove(const KeyType& k)
    {
       _entries.remove(k);
    }

    void touch(const KeyType& k)
    { 
        _entries.remove(k);
        _entries.push_front(k);
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

    /**
     * For the row @p x, increment each column in @p cols1 and @p cols2.
     * @param x the row number
     * @param cols1 column list 1
     * @param cols2 column list 2
     */
    void row_incre(
        KeyType x,
        const std::list<KeyType>& cols1,
        const std::list<KeyType>& cols2
    )
    {
        boost::shared_ptr<RowType> row_data = _row(x);
        _cache_row_dirty_flag.insert(x);

        typename std::list<KeyType>::const_iterator cit;
        for(cit = cols1.begin(); cit != cols1.end(); ++cit)
        {
            typename RowType::iterator it = row_data->find(*cit);
            if(it == row_data->end())
            {
                (*row_data)[*cit].update();
                _currEntries++;
            }
            else
            {
                it->second.update();
            }
        }
        for(cit = cols2.begin(); cit != cols2.end(); ++cit)
        {
            typename RowType::iterator it = row_data->find(*cit);
            if(it == row_data->end())
            {
                (*row_data)[*cit].update();
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
     * update row @p x with @p row_data.
     * @param x row number
     * @param row_data new row data to update
     */
    void update_row(KeyType x, const RowType& row_data)
    {
        typename CacheStorageType::iterator cit = _cache_storage.find(x);
        if(cit != _cache_storage.end())
        {
            // remove old entry count
            boost::shared_ptr<RowType> old_row_data(cit->second);
            size_t old_row_size = old_row_data->size();
            _currEntries = (_currEntries <= old_row_size)?0:(_currEntries - old_row_size);

            // add new entry count
            _evict();
            _currEntries += row_data.size();

            // replace cache
            cit->second.reset(new RowType(row_data));
        }
        else
        {
            // add new entry count
            _evict();
            _currEntries += row_data.size();

            // insert cache
            boost::shared_ptr<RowType> new_row_data(new RowType(row_data));
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

