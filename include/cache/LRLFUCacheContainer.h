#ifndef _LRLFU_CACHE_CONTAINER_H
#define _LRLFU_CACHE_CONTAINER_H

namespace izenelib
{
namespace cache
{

template <class KeyType>
class LRLFUCacheContainer
{
public:
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

    class iterator
    {
    public:
        typedef cache_entry&                        reference;
        typedef cache_entry*                        pointer;
        typedef std::bidirectional_iterator_tag     iterator_category;
        typedef ptrdiff_t                           difference_type;
        typedef iterator                            self;

        inline iterator(pointer curr_node = NULL)
            : _curr_node(curr_node)
        {
        }

        inline reference operator*() const
        {
            return *_curr_node;
        }

        inline pointer operator->() const
        {
            return _curr_node;
        }

        inline self& operator++()
        {
            if (_curr_node)
            {
                if (_curr_node->_newer || !_curr_node->_parent->_hotter)
                {
                    _curr_node = _curr_node->_newer;
                }
                else
                {
                    _curr_node = _curr_node->_parent->_hotter->_oldest;
                }
            }

            return *this;
        }

        inline self operator++(int)
        {
            self tmp = *this;

            if (_curr_node)
            {
                if (_curr_node->_newer || !_curr_node->_parent->_hotter)
                {
                    _curr_node = _curr_node->_newer;
                }
                else
                {
                    _curr_node = _curr_node->_parent->_hotter->_oldest;
                }
            }

            return tmp;
        }

        inline self& operator--()
        {
            if (_curr_node)
            {
                if (_curr_node->_older || !_curr_node->_parent->_colder)
                {
                    _curr_node = _curr_node->_older;
                }
                else
                {
                    _curr_node = _curr_node->_parent->_colder->_newest;
                }
            }

            return *this;
        }

        inline self operator--(int)
        {
            self tmp = *this;

            if (_curr_node)
            {
                if (_curr_node->_older || !_curr_node->_parent->_colder)
                {
                    _curr_node = _curr_node->_older;
                }
                else
                {
                    _curr_node = _curr_node->_parent->_colder->_newest;
                }
            }

            return tmp;
        }

        inline bool operator==(const self& x) const
        {
            return x._curr_node == _curr_node;
        }

        inline bool operator!=(const self& x) const
        {
            return x._curr_node != _curr_node;
        }

    private:
        friend class LRLFUCacheContainer;

        mutable pointer _curr_node;
    };

    LRLFUCacheContainer()
        : _freq_list_head(NULL)
    {
    }

    ~LRLFUCacheContainer()
    {
        _clear();
    }

    inline iterator begin()
    {
        if (_freq_list_head)
            return iterator(_freq_list_head->_oldest);
        else
            return iterator();
    }

    inline iterator end()
    {
        return iterator();
    }

    inline bool empty() const
    {
        return !_freq_list_head;
    }

    inline iterator _firstInsert(const KeyType& key)
    {
        struct cache_entry *new_entry = new cache_entry(key);

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

        return iterator(new_entry);
    }

    inline void _replace(iterator& entry)
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
        parent->_hotter->insert(entry._curr_node);
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

    inline bool _evict(KeyType& key)
    {
        if (!_freq_list_head)
            return false;

        if (!_freq_list_head->evict(key))
        {
            struct freq_node *freq_list_head = _freq_list_head->_hotter;
            delete _freq_list_head;
            _freq_list_head = freq_list_head;
            if (_freq_list_head)
                _freq_list_head->_colder = NULL;
        }

        return true;
    }

    inline void _clear()
    {
        while (_freq_list_head)
        {
            _freq_list_head->clear();
            struct freq_node *freq_list_head = _freq_list_head->_hotter;
            delete _freq_list_head;
            _freq_list_head = freq_list_head;
        }
    }

private:
    struct freq_node *_freq_list_head;
};

}
}

#endif
