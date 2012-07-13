// Components for manipulating sequences of characters -*- C++ -*-

// Copyright (C) 2008 Ofer Samocha & Sunny Marueli
//
#ifndef __STI_SSET_H__
#define __STI_SSET_H__

#include "stree.h"
namespace sti
{
    template<
          class Key
        , class Traits = std::less<Key>
        , class Allocator = std::allocator<Key>
        , int BN = 48
        , class key_policy = implementation::default_stree_key_policy<Key>
        , class gist_traits = implementation::default_gist_traits<Key>
    >
    class sset
    {
    public:
        // typedefs from std::set
        typedef Allocator allocator_type;
        typedef Key key_type;
        typedef Key value_type;
        typedef Traits key_compare;
        typedef key_compare value_compare;

        typedef typename Allocator::size_type size_type;
        typedef typename Allocator::pointer pointer;
        typedef typename Allocator::const_pointer const_pointer;
        typedef typename Allocator::reference reference;
        typedef typename Allocator::const_reference const_reference;
        typedef typename Allocator::difference_type difference_type;

        // And a few of our own
        const static int block_size = BN;
        typedef sset<Key,Traits,Allocator,BN, key_policy, gist_traits> this_type;

    private:
        typedef typename implementation::stree<
            key_type, value_type, allocator_type, key_compare,
            stree_key_extractor<key_type, value_type>, 
            block_size, key_policy, boost::true_type, gist_traits
        > STree;


    public:
        typedef typename STree::iterator iterator;
        typedef typename STree::const_iterator const_iterator;
        typedef typename STree::reverse_iterator reverse_iterator;
        typedef typename STree::const_reverse_iterator const_reverse_iterator;

    public:
        sset(Traits = Traits(), allocator_type al = allocator_type())
            : _stree(al)
        {}
        ~sset() {}

        template<class InputIterator>
        sset(InputIterator first, InputIterator last, Traits = Traits(), allocator_type al = allocator_type())
            : _stree(al)
        {
            insert(first, last);
        }

    public:
        // Delegate everything to stree
        iterator begin()                                  { return _stree.begin(); }
        const_iterator begin() const                      { return _stree.begin(); }
        iterator end()                                    { return _stree.end(); }
        const_iterator end() const                        { return _stree.end(); }

        const_reverse_iterator rbegin( ) const            { return _stree.rbegin(); }
        reverse_iterator rbegin( )                        { return _stree.rbegin(); }
        const_reverse_iterator rend( ) const              { return _stree.rend(); }
        reverse_iterator rend( )                          { return _stree.rend(); }

        const_iterator cbegin() const                     { return _stree.begin(); }
        const_iterator cend() const                       { return _stree.end(); }
        const_reverse_iterator crbegin( ) const           { return _stree.rbegin(); }
        const_reverse_iterator crend( ) const             { return _stree.rend(); }


        void clear()                                      { _stree.clear(); }
        bool empty() const                                { return _stree.empty(); }
        iterator erase(iterator _Where)                   { return _stree.erase(_Where); }
        iterator erase(iterator _First, iterator _Last)   { return _stree.erase(_First, _Last); }
        size_type erase(const key_type& _Key)             { return _stree.erase(_Key); }
        iterator find(const key_type& _Key)               { return _stree.find(_Key);    }
        const_iterator find(const key_type& _Key) const   { return _stree.find(_Key);    }
        allocator_type get_allocator() const              { return allocator_type();    }
        std::pair <iterator, bool> 
            insert(const value_type& _Val)                { return _stree.insert(_Val);    }
        iterator insert(
            iterator _Where,const value_type& _Val)       { return _stree.insert(_Where, _Val); }
        key_compare key_comp( ) const                     { return key_compare();    }
        const_iterator 
            lower_bound(const Key& _Key) const            { return _stree.lower_bound(_Key);}
        iterator lower_bound(const Key& _Key)             { return _stree.lower_bound(_Key);    }
        const_iterator 
            upper_bound(const Key& _Key) const            { return _stree.upper_bound(_Key); }
        iterator upper_bound(const Key& _Key)             { return _stree.upper_bound(_Key); }
        size_type size( ) const                           { return _stree.size(); }
        value_compare value_comp( ) const                 { return value_compare(); }
        std::pair<iterator, iterator> 
            equal_range( const key_type& key )            { return _stree.equal_range(key); }

        void swap(this_type& _Right)
        {
            _stree.swap(_Right._stree);
        }

        size_type count(const key_type& key) const        { return _stree.count(key); }

        template<class InputIterator>
        void insert(InputIterator First, InputIterator Last)  
        {
            for (InputIterator it = First; it != Last; ++it)
                insert(*it);
        }

        size_type max_size( ) const
        {
            return ((size_type)1<<(sizeof(size_type)-1)/sizeof(key_type))*2;
        }

        this_type operator=(const this_type& c2)
        {
            this_type t(c2);
            this->swap(t);
            return *this;
        }

        bool operator==(const this_type& o) const { return _stree == o._stree; }
        bool operator!=(const this_type& o) const { return _stree != o._stree; }
    private:
        STree _stree;
    };
}

namespace std
{
    template <class Key, class Traits, class Allocator, int BN>
    inline void swap(sti::sset<Key, Traits, Allocator, BN>& x, sti::sset<Key, Traits, Allocator, BN>& y)
    {
        x.swap(y);
    }
}
#endif // __STI_SSET_H__
