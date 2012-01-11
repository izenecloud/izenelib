#ifndef IZENELIB_AM_CACHE_POLICY_H
#define IZENELIB_AM_CACHE_POLICY_H

#include <types.h>

#include <3rdparty/am/stx/btree_map>
#include <3rdparty/am/stx/btree_set>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <util/ThreadModel.h>

#include <set>
#include <map>
#include <list>

NS_IZENELIB_AM_BEGIN

namespace detail
{

template <class KeyType,
          class LockType = izenelib::util::ReadWriteLock>
class policy_lrlfu
{
    struct freq_node;
    struct cache_entry
    {
        struct cache_entry *_newer, *_older;
        struct freq_node *_parent;
        KeyType _key;

        cache_entry(const KeyType& k)
            : _newer(NULL), _older(NULL)
            , _parent(NULL)
            , _key(k)
        {
        }
    };

    struct freq_node
    {
        unsigned long _freq, _count;
        struct cache_entry *_newest, *_oldest;
        struct freq_node *_hotter, *_colder;

        freq_node(unsigned long freq = 1)
            : _freq(freq)
            , _count(0)
            , _newest(NULL), _oldest(NULL)
            , _hotter(NULL), _colder(NULL)
        {
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
            ++_count;
            new_entry->_parent = this;
        }

        bool evict(KeyType& k)
        {
            k = _oldest->_key;
            struct cache_entry *oldest = _oldest->_newer;
            delete _oldest;
            _oldest = oldest;
            --_count;

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

    typedef ::rde::hash_map<KeyType, struct cache_entry *> backentry_type;
    struct freq_node *_freq_list_head;
    backentry_type _backEntries;

    typedef izenelib::util::ScopedReadLock<LockType> ScopedReadLock;
    typedef izenelib::util::ScopedWriteLock<LockType> ScopedWriteLock;
    mutable LockType lock_;

public:
    policy_lrlfu()
        : _freq_list_head(NULL)
    {
    }

    ~policy_lrlfu()
    {
        clear();
    }

    void remove(const KeyType& k)
    {
        ScopedWriteLock lock(lock_);

        typename backentry_type::iterator it = _backEntries.find(k);
        if (it != _backEntries.end())
        {
            _removeEntry(it->second);
        }
    }

    void touch(const KeyType& k)
    {
        ScopedWriteLock lock(lock_);

        typename backentry_type::iterator it = _backEntries.find(k);
        if (it != _backEntries.end())
        {
            _touchEntry(it->second);
        }
        else
        {
            struct cache_entry *new_entry = new cache_entry(k);
            _insertEntry(new_entry);
        }
    }

    void clear()
    {
        ScopedWriteLock lock(lock_);

        while (_freq_list_head)
        {
            _freq_list_head->clear();
            struct freq_node *freq_list_head = _freq_list_head->_hotter;
            delete _freq_list_head;
            _freq_list_head = freq_list_head;
        }
        _backEntries.clear();
    }

    size_t size() const
    {
        ScopedReadLock lock(lock_);

        size_t count = 0;
        for (struct freq_node *it = _freq_list_head; it != NULL; it = it->_hotter)
        {
            count += sizeof(struct cache_entry) * it->_count;
            count += sizeof(struct freq_node);
        }
        count += _backEntries.size() * (sizeof(KeyType) + sizeof(struct cache_entry *));
        return count;
    }

    bool evict(KeyType& k)
    {
        ScopedWriteLock lock(lock_);

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

private:
    void _insertEntry(struct cache_entry *entry)
    {
        const KeyType key = entry->_key;
        if (!_freq_list_head || _freq_list_head->_freq != 1)
        {
            struct freq_node *new_freq_node = new freq_node();
            new_freq_node->_hotter = _freq_list_head;
            if (_freq_list_head)
                _freq_list_head->_colder = new_freq_node;
            _freq_list_head = new_freq_node;
        }
        _freq_list_head->insert(entry);
        entry->_parent = _freq_list_head;
        _backEntries.insert(rde::pair<KeyType, struct cache_entry *>(key, entry));
    }

    void _touchEntry(struct cache_entry *entry)
    {
        struct freq_node *parent = entry->_parent;
        if (parent->_count == 1 && (!parent->_hotter || parent->_hotter->_freq != parent->_freq + 1))
        {
            parent->_freq++;
            return;
        }
        if (entry->_newer)
            entry->_newer->_older = entry->_older;
        else
            parent->_newest = entry->_older;

        if (entry->_older)
            entry->_older->_newer = entry->_newer;
        else
            parent->_oldest = entry->_newer;

        if (!parent->_hotter || (parent->_hotter->_freq != parent->_freq + 1))
        {
            struct freq_node *hotter = new freq_node(parent->_freq + 1);
            hotter->_colder = parent;
            hotter->_hotter = parent->_hotter;
            if (parent->_hotter)
                parent->_hotter->_colder = hotter;
            parent->_hotter = hotter;
        }
        parent->_hotter->insert(entry);
        entry->_parent = parent->_hotter;
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

    void _removeEntry(struct cache_entry *entry)
    {
        const KeyType key = entry->_key;
        struct freq_node *parent = entry->_parent;
        if (entry->_newer)
            entry->_newer->_older = entry->_older;
        else
            parent->_newest = entry->_older;

        if (entry->_older)
            entry->_older->_newer = entry->_newer;
        else
            parent->_oldest = entry->_newer;

        delete entry;
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
        {
            parent->_count--;
        }

        _backEntries.erase(key);
    }
};

template <class KeyType,
          class LockType = izenelib::util::ReadWriteLock>
class policy_lfu
{
    typedef ::stx::btree_set<KeyType> keyset_type;
    typedef ::stx::btree_map<unsigned long, keyset_type> entry_type;
    typedef ::rde::hash_map<KeyType, unsigned long> backentry_type;
    entry_type _entries;
    backentry_type _backEntries;

    typedef izenelib::util::ScopedReadLock<LockType> ScopedReadLock;
    typedef izenelib::util::ScopedWriteLock<LockType> ScopedWriteLock;
    mutable LockType lock_;

public:
    void remove(const KeyType& k)
    {
        ScopedWriteLock lock(lock_);

        typename backentry_type::iterator it = _backEntries.find(k);
        if (it != _backEntries.end())
        {
            keyset_type& pad = _entries[it->second];
            pad.erase(k);
            if (pad.empty())
            {
                _entries.erase(it->second);
            }
            _backEntries.erase(k);
        }
    }

    void touch(const KeyType& k)
    {
        ScopedWriteLock lock(lock_);

        unsigned long& ref = _backEntries[k];
        if (ref)
        {
            keyset_type& pad = _entries[ref];
            pad.erase(k);
            if (pad.empty())
            {
                _entries.erase(ref);
            }
        }
        keyset_type& new_pad = _entries[++ref];
        new_pad.insert(k);
    }

    void clear()
    {
        ScopedWriteLock lock(lock_);
        _entries.clear();
        _backEntries.clear();
    }

    size_t size() const
    {
        ScopedReadLock lock(lock_);

        size_t count = 0;
        for (typename entry_type::iterator it = _entries.begin();
                it != _entries.end(); ++it)
        {
            count += sizeof(KeyType) * it.data().size();
            count += sizeof(unsigned long);
        }
        for (typename backentry_type::iterator it = _backEntries.begin();
                it != _backEntries.end(); ++it)
        {
            count += sizeof(KeyType);
            count += sizeof(unsigned long);
        }
        return count;
    }

    bool evict(KeyType& k)
    {
        ScopedWriteLock lock(lock_);

        if (_entries.empty()) return false;

        typename entry_type::iterator iter = _entries.begin();

        keyset_type& pad = iter.data();
        k = *(pad.begin());
        pad.erase(k);
        if (pad.empty())
            _entries.erase(iter);
        _backEntries.erase(k);
        return true;
    }
};

template <class KeyType,
          class LockType = izenelib::util::ReadWriteLock>
class policy_lru
{
    std::list<KeyType> _entries;

    typedef izenelib::util::ScopedReadLock<LockType> ScopedReadLock;
    typedef izenelib::util::ScopedWriteLock<LockType> ScopedWriteLock;
    mutable LockType lock_;

public:
    void remove(const KeyType& k)
    {
        ScopedWriteLock lock(lock_);
        _entries.remove(k);
    }

    void touch(const KeyType& k)
    {
        ScopedWriteLock lock(lock_);
        _entries.remove(k);
        _entries.push_front(k);
    }

    void clear()
    {
        ScopedWriteLock lock(lock_);
        _entries.clear();
    }

    size_t size() const
    {
        ScopedReadLock lock(lock_);
        return _entries.size() * sizeof(KeyType);
    }

    bool evict(KeyType& k)
    {
        ScopedWriteLock lock(lock_);

        if (_entries.empty())
            return false;
        k = _entries.back();
        _entries.remove(k);
        return true;
    }
};

}

NS_IZENELIB_AM_END

#endif
