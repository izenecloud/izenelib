#ifndef IZENELIB_AM_CACHE_POLICY_H
#define IZENELIB_AM_CACHE_POLICY_H

#include <types.h>
 
#include <3rdparty/am/stx/btree_map>
#include <3rdparty/am/stx/btree_set>
#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <set>
#include <map>
#include <list>
 
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
    policy_lfu_nouveau()
    {
        _freq_list_head = NULL;
    }

    ~policy_lfu_nouveau()
    {
        clear();
    }

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
        if (_backEntries.find(k) != _backEntries.end())
        {
            struct cache_entry *ref = _backEntries[k];
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
        if (_backEntries.find(k) != _backEntries.end())
        {
            struct cache_entry *ref = _backEntries[k];
            struct freq_node *parent = ref->_parent;
            if (parent->_count == 1 && (!parent->_hotter || parent->_hotter->_freq != parent->_freq + 1))
            {
                parent->_freq++;
                return;
            }
            if (ref->_newer)
                ref->_newer->_older = ref->_older;
            else
                parent->_newest = ref->_older;

            if (ref->_older)
                ref->_older->_newer = ref->_newer;
            else
                parent->_oldest = ref->_newer;

            if (!parent->_hotter || (parent->_hotter->_freq != parent->_freq + 1))
            {
                struct freq_node *hotter = new freq_node(parent->_freq + 1);
                hotter->_colder = parent;
                hotter->_hotter = parent->_hotter;
                if (parent->_hotter)
                    parent->_hotter->_colder = hotter;
                parent->_hotter = hotter;
            }
            parent->_hotter->insert(ref);
            ref->_parent = parent->_hotter;
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
            struct freq_node *freq_list_head = _freq_list_head->_hotter;
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
 
NS_IZENELIB_AM_END

 #endif

