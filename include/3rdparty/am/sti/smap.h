// Components for manipulating sequences of characters -*- C++ -*-

// Copyright (C) 2008 Ofer Samocha & Sunny Marueli
//
#ifndef __STI_SMAP_H__
#define __STI_SMAP_H__

#include "stree.h"
#include <boost/mpl/if.hpp>
#include <boost/iterator/indirect_iterator.hpp>

namespace sti
{
    namespace implementation
    {
        template<class T, bool b>
            struct is_small_type_helper : boost::false_type    {};
        template<class T>
            struct is_small_type_helper<T, true> : boost::true_type {};
        template<class T>
            struct is_small_type: public is_small_type_helper<T, sizeof(T) <= sizeof(long)> {};

        template <class IT, typename Value>
        class smap_iter_helper : 
            public boost::iterator_adaptor<
                                smap_iter_helper<IT, Value>, 
                                IT, Value
                            >
        {
            typedef typename  boost::iterator_adaptor<smap_iter_helper<IT, Value>, IT,Value> super_t;
            friend class boost::iterator_core_access;
        public:
            smap_iter_helper() {}

            smap_iter_helper(IT it)
                : super_t(it) {}

            template <class OtherIt, class OtherValue>
            smap_iter_helper
                (const smap_iter_helper<OtherIt, OtherValue>& other
                , typename boost::enable_if<boost::is_convertible<OtherIt,IT> >::type* = 0)
                : super_t(other.base()) 
            {}
        };

        template<class Key, class Type>
        struct default_smap_value_policy
            : implementation::is_small_type<std::pair<Key, Type> >::type{};

        template< class Key
                , class Type
                , class Traits
                , class Allocator
                , int BN
                , class key_policy
                , class value_policy
                , class gist_traits
        >
        class smap_base 
            : public implementation::stree<
                        Key, std::pair<const Key, Type>, Allocator, Traits, 
                        stree_key_extractor<Key, std::pair<const Key, Type> >, 
                        BN, key_policy, value_policy, gist_traits
                     >
        {
            typedef smap_base<Key, Type, Traits, Allocator, BN, key_policy, value_policy, gist_traits> this_type;
            typedef typename implementation::stree<
                Key, std::pair<const Key, Type>,  Allocator, Traits, 
                stree_key_extractor<Key, std::pair<const Key, Type> >, 
                BN, key_policy, value_policy, gist_traits> super_t;
        protected:
            typedef typename super_t::iterator stree_iterator;
            typedef typename super_t::const_iterator const_stree_iterator;

            template<class IT, class Value>
            class iter_helper 
                : public boost::iterator_adaptor<
                            iter_helper<IT, Value>
                          , implementation::smap_iter_helper<IT, Value>
                        >
            {
                friend class smap_base<Key, Type, Traits, Allocator, BN, key_policy, value_policy, gist_traits>;
                typedef iter_helper<IT, Value> this_type;
                typedef typename 
                    boost::iterator_adaptor<
                              iter_helper<IT, Value>
                            , implementation::smap_iter_helper<IT, Value>
                        > super_t;

            private:
                explicit iter_helper(IT it) : super_t(it) {}

                const IT& stree_iter() const
                {
                    const implementation::smap_iter_helper<IT, Value>& i = super_t::base();
                    return i.base();
                }

            public:

                iter_helper() {}

                // Convert const/non-const
                template<class OI, class OV>
                iter_helper(const iter_helper<OI, OV>& o, typename boost::enable_if<boost::is_convertible<OV, Value> > ::type* = 0)
                    : super_t(o.base())
                {}
            };


        public:
            const static int block_size = BN;
            typedef typename super_t::value_type value_type;
            typedef Traits key_compare;
            typedef Key key_type;
            typedef Allocator allocator_type;

            typedef Type mapped_type;
            typedef key_compare value_compare; // XX!X
            typedef iter_helper<stree_iterator, value_type> iterator;
            typedef iter_helper<const_stree_iterator, const value_type> const_iterator;

            typedef typename boost::reverse_iterator<iterator> reverse_iterator;
            typedef typename boost::reverse_iterator<const_iterator> const_reverse_iterator;

            typedef typename Allocator::size_type size_type;
            typedef typename Allocator::pointer pointer;
            typedef typename Allocator::const_pointer const_pointer;
            typedef typename Allocator::reference reference;
            typedef typename Allocator::const_reference const_reference;
            typedef typename Allocator::difference_type difference_type;

        public:
            smap_base(Traits, Allocator al)
                : super_t(al)
            {
            }

            smap_base(const this_type& o)
                : super_t((Allocator)o)
            {
                for (iterator it = o.begin(); it != o.end(); ++it)
                    insert(*it);
            }

            const_iterator cbegin( ) const                    { return const_iterator(super_t::begin()); }
            const_iterator cend( ) const                      { return const_iterator(super_t::end()); }

            iterator begin( )                                 { return iterator(super_t::begin()); }
            iterator end( )                                   { return iterator(super_t::end()); }
            const_iterator begin() const                      { return cbegin(); }
            const_iterator end() const                        { return cend(); }

            const_reverse_iterator crbegin( ) const           { return const_reverse_iterator(end()); }
            const_reverse_iterator crend( ) const             { return const_reverse_iterator(begin()); }

            reverse_iterator rbegin( )                        { return reverse_iterator(end()); }
            reverse_iterator rend( )                          { return reverse_iterator(begin()); }

            const_reverse_iterator rbegin( ) const            { return crbegin(); }
            const_reverse_iterator rend( ) const              { return crend(); }

            iterator find(const key_type& k)                  { return iterator(super_t::find(k)); }
            const_iterator find(const key_type& k) const      { return const_iterator(super_t::find(k)); }

            iterator lower_bound( const key_type& key )       { return iterator(super_t::lower_bound(key)); }
            iterator upper_bound( const key_type& key )       { return iterator(super_t::upper_bound(key)); }
            //void swap(this_type& _Right)                    { super_t::swap(_Right._stree); }

            iterator erase(iterator it)
            {
                stree_iterator sit = it.stree_iter();
                return iterator(super_t::erase(sit));
            }

            size_type erase(const key_type& k)                { return super_t::erase(k);    }

            void erase( iterator start, iterator end )        { super_t::erase(start, end); }
            iterator insert( 
                iterator i, const value_type& p )
            { 
                return iterator(super_t::insert(i.stree_iter(), p)); 
            }

            std::pair<iterator, iterator> 
                equal_range( const key_type& key )
            {
                std::pair<stree_iterator, stree_iterator> r = super_t::equal_range(key);
                return std::make_pair(iterator(r.first), iterator(r.second));
            }

            template<typename input_iterator>
            void insert(input_iterator start, input_iterator end )
            {
                for (input_iterator i = start; i != end; ++i)
                    insert(*i);
            }

            std::pair <iterator, bool>
                insert(const value_type& _Val)        
            {
                // OPTIMIZE
                // This may result in 2 tree-traversals: one for the lower-bound, and another for the insert.
                // Usually, there will be enough room to insert new item into the bottom node
                stree_iterator it = super_t::lower_bound(_Val.first);

                if (it != super_t::end() && !super_t::less(_Val.first, it->first))
                    return std::make_pair(iterator(it), false);
                const value_type& e = _Val;

                if (it != super_t::begin())
                    --it;

                stree_iterator r = super_t::insert(it, e);
                return std::make_pair(iterator(r), true);
            }

            mapped_type& at(const key_type& k)
            {
                iterator f = find(k);
                if (f==this->end())
                    throw std::out_of_range("smap::at");
                return f->second;
            }
            const mapped_type& at(const key_type& k) const 
            {
                const_iterator i = this->find(k);
                if (i == this->end())
                    throw std::out_of_range("smap::at");
                return i->second;
            }

            mapped_type& operator[](const key_type& k)
            {
                iterator f = find(k);
                if (f==this->end())
                    boost::tie(f, boost::tuples::ignore) = insert(value_type(k, Type()));
                return f->second;
            }

            const mapped_type& operator[](const key_type& k) const { return at(k); }

            void clear()
            {
                for (iterator i = this->begin(); i != this->end();i = this->begin())
                    this->erase(i);
            }

            bool operator==(const this_type& o) const
            {
                const_iterator it1, it2;
                if (this->size() != o.size())
                    return false;
                for (it1 = this->begin(), it2=o.begin(); it1!= this->end() && it2!= o.end(); ++it1, ++it2)
                {
                    if (it1->first != it2->first)
                        return false;
                    if (!this->equal_elements(it1->second, it2->second))
                        return false;
                }
                return true; 
            }

            bool operator!=(const this_type& o) const        { return !operator==(o); }

        protected:
            iterator make_iterator(stree_iterator it)                      { return iterator(it);    }
            const_iterator make_iterator(const_stree_iterator it) const    { return const_iterator(it);    }

            bool equal_elements(const Type& a,  const Type& b) const
            {
                return a == b;
            }

        };
    }

    template<
          class Key
        , class Type
        , class Traits = std::less<Key>
        , class Allocator = std::allocator<std::pair <const Key, Type> >
        , int BN = 48
        , class key_policy       = implementation::default_stree_key_policy<Key>
        , class value_policy     = implementation::default_smap_value_policy<Key, Type>
        , class gist_traits      = implementation::default_gist_traits<Key>
    >
    class smap : public implementation::smap_base<Key, Type, Traits, Allocator, BN, key_policy, value_policy, gist_traits >
    {
        typedef implementation::smap_base<Key, Type, Traits, Allocator, BN, key_policy,  value_policy, gist_traits > base;
        typedef smap<Key, Type, Traits, Allocator, BN, key_policy, value_policy, gist_traits> this_type;
    public:
        typedef Key key_type;
        typedef Type mapped_type;
        typedef typename base::const_iterator const_iterator;
        typedef typename base::iterator iterator;
        typedef typename base::const_reverse_iterator const_reverse_iterator;
        typedef typename base::reverse_iterator reverse_iterator;
        typedef typename base::allocator_type allocator_type;
        typedef typename base::size_type size_type;

        smap(Traits t = Traits(), Allocator al = Allocator())
            : base(t, al)
        {}

        smap(const this_type& other)
            : base(other)
        {
        }
        ~smap()
        {
            this->clear();
        }

        template<class InputIterator>
        smap(InputIterator first, InputIterator last, Traits t = Traits(), Allocator al = Allocator())
            : base(t, al)
        {
            this->insert(first, last);
        }
    };
}

namespace std
{
    template<class Key, class Type, class Traits, class Allocator, int BN, class key_policy>
    inline void
        swap(sti::smap<Key, Type, Traits, Allocator, BN, key_policy>& x, sti::smap<Key, Type, Traits, Allocator, BN, key_policy>& y)
    {
        x.swap(y);
    }
}
#endif // __STI_SMAP_H__
