// Components for manipulating sequences of characters -*- C++ -*-

// Copyright (C) 2008 Ofer Samocha & Sunny Marueli
//
#ifndef __STI_STREE_H__
#define __STI_STREE_H__

#include <limits>
#include <memory>
#include <functional>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/call_traits.hpp>
#include <boost/preprocessor/iteration/local.hpp>

// Include TR1 stuff
#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/assert.hpp>


namespace sti
{
    template<typename Key, typename value_type>
    struct stree_key_extractor
    {
        const Key& operator()(const value_type& v) const { return v.first; }
    };

    template<typename Key>
    struct stree_key_extractor<Key, Key>
    {
        const Key& operator()(const Key& v) const { return v; }
    };

    namespace implementation
    {
        template<typename T>
        struct default_gist_traits
        {
            typedef T type;

            size_t operator()(typename boost::call_traits<T>::param_type t) const
            {
                return 0;
            }
        };

        template<class C, size_t>
        struct str_to_size_t_helper_impl
        {
            size_t operator()(const C* c, size_t sz) const
            {
                if (sz > sizeof(size_t)/sizeof(C))
                    sz = sizeof(size_t)/sizeof(C);
                size_t i = 0;
                size_t r = 0;
                for (;i < sz; ++i)
                {
                    r <<= 8*sizeof(C);
                    r += (size_t)c[i];
                }
                return r;
            }
        };
    
        template<class C>
        struct str_to_size_t_helper_impl<C, 4>
        {
            size_t operator()(const C* c, size_t sz) const
            {
                const static size_t bits_per_c = 8*sizeof(C);
                switch(sz)
                {
                case 0: return 0;
                case 1: return (size_t)c[0]<<(3*bits_per_c);
                case 2: return ((size_t)c[0]<<(3*bits_per_c)) + ((size_t)c[1]<<(2*bits_per_c));
                case 3: return ((size_t)c[0]<<(3*bits_per_c)) + ((size_t)c[1]<<(2*bits_per_c)) + ((size_t)c[2]<<(1*bits_per_c));
                case 4:
                default: return ((size_t)c[0]<<(3*bits_per_c)) + ((size_t)c[1]<<(2*bits_per_c)) + ((size_t)c[2]<<(1*bits_per_c)) + (size_t)c[3];
                }
            }
        };

        template<class C>
        struct str_to_size_t_helper_impl<C, 8>
        {
            size_t operator()(const C* c, size_t sz) const
            {
                const static size_t bits_per_c = 8*sizeof(C);
                switch(sz)
                {
                case 0: return 0;
                case 1: return  (size_t)c[0]<<(7*bits_per_c);
                case 2: return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c));
                case 3: return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c)) + ((size_t)c[2]<<(5*bits_per_c));
                case 4: return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c)) + ((size_t)c[2]<<(5*bits_per_c)) 
                                        + ((size_t)c[3]<<(4*bits_per_c));
                case 5: return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c)) + ((size_t)c[2]<<(5*bits_per_c)) 
                                        + ((size_t)c[3]<<(4*bits_per_c)) + ((size_t)c[4]<<(3*bits_per_c));
                case 6: return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c)) + ((size_t)c[2]<<(5*bits_per_c)) 
                                        + ((size_t)c[3]<<(4*bits_per_c)) + ((size_t)c[4]<<(3*bits_per_c)) + ((size_t)c[5]<<(2*bits_per_c));
                case 7: return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c)) + ((size_t)c[2]<<(5*bits_per_c)) 
                                        + ((size_t)c[3]<<(4*bits_per_c)) + ((size_t)c[4]<<(3*bits_per_c)) + ((size_t)c[5]<<(2*bits_per_c)) 
                                        + ((size_t)c[6]<<(1*bits_per_c));
                case 8:
                default:return ((size_t)c[0]<<(7*bits_per_c)) + ((size_t)c[1]<<(6*bits_per_c)) + ((size_t)c[2]<<(5*bits_per_c)) 
                                        + ((size_t)c[3]<<(4*bits_per_c)) + ((size_t)c[4]<<(3*bits_per_c)) + ((size_t)c[5]<<(2*bits_per_c)) 
                                        + ((size_t)c[6]<<(1*bits_per_c)) + (size_t)c[7];
                }
            }
        };


        template<class C>
        size_t str_to_size_t_helper(const C* c, size_t sz)
        {
            return str_to_size_t_helper_impl<C, sizeof(sz)/sizeof(C)>()(c,sz);
        }

        template<typename C, typename T, typename A>
        struct default_gist_traits<std::basic_string<C, T, A> >
        {
            typedef std::basic_string<C, T, A> type;

            size_t operator()(const type& t) const
            {
                return implementation::str_to_size_t_helper(t.c_str(),  t.length());
            }
        };

        template<class K>
        struct default_stree_key_policy: public boost::is_pod<K>::type {};

        template<class Key, class GIST_TRAITS, class>
        struct KeyWithGistImpl
        {
            static const bool has_gist = true;
            KeyWithGistImpl(const Key& k)
                : v(k)
            {
                gist = GIST_TRAITS()(k);
            }
            size_t gist;
            const Key& v;
        };

        template<class Key, class GIST_TRAITS>
        struct KeyWithGistImpl<Key, GIST_TRAITS, boost::true_type>
        {
            static const bool has_gist = false;
            KeyWithGistImpl(const Key& k) : v(k) {}
            const Key& v;
        };

        template<class Key, class GIST_TRAITS, class key_policy>
        struct KeyWithGist : public KeyWithGistImpl<Key, GIST_TRAITS, typename key_policy::type>
        {
            typedef KeyWithGistImpl<Key, GIST_TRAITS, typename key_policy::type> base;
            KeyWithGist(const Key& k) : base(k) {}
        };

        template<class Key, class GIST_TRAITS, class Allocator, class S>
        struct KeyWrapperImpl : protected Allocator
        {
            static const bool has_gist = true;
            typedef KeyWrapperImpl<Key, GIST_TRAITS, Allocator, S> this_type;

            KeyWrapperImpl() :v(0) {}
            ~KeyWrapperImpl() { reset(); }

            explicit KeyWrapperImpl(const Key& k)
                : v(0)
            {
                v = this->allocate(1);
                new(v) Key(k);
                gist = GIST_TRAITS()(k);
            }

            this_type& operator=(const this_type& k)
            {
                if (v)
                    *v = *k.v;
                else
                {
                    v = this->allocate(1);
                    new(v) Key();
                }
                gist = k.gist;
                return *this;
            }

            this_type& operator=(const Key& k)
            {
                if (v)
                    *v = k;
                else
                {
                    v = this->allocate(1);
                    new(v) Key(k);
                }
                gist = GIST_TRAITS()(k); // XXX
                return *this;
            }
            template<class key_policy>
            this_type& operator=(const KeyWithGist<Key, GIST_TRAITS, key_policy>& k)
            {
                if (v)
                    *v = k.v;
                else
                {
                    v = this->allocate(1);
                    new(v) Key(k.v);
                }
                gist = k.gist;
                return *this;
            }

            size_t gist;
            Key* v;

            void reset()
            {
                if (v)
                {
                    this->destroy(v);
                    this->deallocate(v, 1);
                }
                v = 0;
            }
        };

        template<class Key, class GIST_TRAITS, class Allocator>
        struct KeyWrapperImpl<Key, GIST_TRAITS, Allocator, boost::true_type> : protected Allocator
        {
            static const bool has_gist = false;

            typedef KeyWrapperImpl<Key, GIST_TRAITS, Allocator, boost::true_type> this_type;

            KeyWrapperImpl() :v(0) {}
            ~KeyWrapperImpl() { }

            explicit KeyWrapperImpl(const Key& k) : v(k) { }
            this_type& operator=(const this_type& k)
            {
                v = k.v;
                return *this;
            }

            this_type& operator=(const Key& k)
            {
                v = k;
                return *this;
            }

            template<class key_policy>
            this_type& operator=(const KeyWithGist<Key, GIST_TRAITS, key_policy>& k)
            {
                v = k.v;
                return *this;
            }

            Key v;
        };

        template<class Key, class GIST_TRAITS, class Allocator, class key_policy >
        struct KeyWrapper : public KeyWrapperImpl<Key, GIST_TRAITS, Allocator, typename key_policy::type >
        {
            typedef KeyWrapperImpl<Key, GIST_TRAITS, Allocator, typename key_policy::type > base;
            typedef KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy> this_type;
            KeyWrapper() {}
            ~KeyWrapper() {}
            explicit KeyWrapper(const Key& k): base(k) {}
            this_type& operator=(const KeyWithGist<Key, GIST_TRAITS, key_policy>& k)
            {
                base::operator=(k);
                return *this;
            }

        };

        template<class Key>
        const Key& extract_key(const Key& k) { return k; }

        template<class Key, class GIST_TRAITS, class key_policy>
        const Key& extract_key(const KeyWithGist<Key, GIST_TRAITS, key_policy>& k) { return k.v; }

        template<class Key, class GIST_TRAITS, class Allocator>
        const Key& extract_key(const KeyWrapperImpl<Key, GIST_TRAITS, Allocator, boost::false_type>& k) { return *k.v; }

        template<class Key, class GIST_TRAITS, class Allocator>
        const Key& extract_key(const KeyWrapperImpl<Key, GIST_TRAITS, Allocator, boost::true_type>& k) { return k.v; }

        template<class Key, class GIST_TRAITS, class Allocator, class key_policy>
        const Key& extract_key(const KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>& k)
        { 
            return extract_key(static_cast<const typename KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>::base& >(k));
        }

        template<class Key, class LESS>
        bool compare_key(const Key& l, const Key& r, LESS c)
        {
            return c(l, r);
        }

        template<class KeyWrap1, class KeyWrap2, class LESS> 
        bool compare_key_helper(const KeyWrap1& l, const KeyWrap2& r, LESS comp, boost::false_type)
        {
            return comp(extract_key(l), extract_key(r));
        }

        template<class KeyWrap1, class KeyWrap2, class LESS> 
        bool compare_key_helper(const KeyWrap1& l, const KeyWrap2& r, LESS comp, boost::true_type)
        {
            if (l.gist==r.gist)
                return comp(extract_key(l), extract_key(r));
            else 
                return l.gist<r.gist;
        }

        template<class Key, class GIST_TRAITS, class Allocator, class LESS, class key_policy>
        bool compare_key(const KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>& l, const KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>& r, LESS comp)
        {
            typedef typename 
                    boost::integral_constant<
                          bool, 
                          KeyWrapper<Key,GIST_TRAITS, Allocator, key_policy>::has_gist
                    >::type G;
            return compare_key_helper(l, r, comp, G());
        }

        template<class Key, class GIST_TRAITS, class LESS, class key_policy>
        bool compare_key(const KeyWithGist<Key, GIST_TRAITS, key_policy>& l, const KeyWithGist<Key, GIST_TRAITS, key_policy>& r, LESS comp)
        {
            typedef typename
                    boost::integral_constant<
                          bool,
                          KeyWithGist<Key, GIST_TRAITS, key_policy>::has_gist
                    >::type G;
            return compare_key_helper(l, r, comp, G());
        }

        template<class Key, class GIST_TRAITS, class Allocator, class LESS, class key_policy>
        bool compare_key(const KeyWithGist<Key, GIST_TRAITS, key_policy>& l, const KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>& r, LESS comp)
        {
            typedef typename
                    boost::integral_constant<
                          bool,
                          KeyWithGist<Key, GIST_TRAITS, key_policy>::has_gist && KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>::has_gist
                    >::type G;
                
            return compare_key_helper(l, r, comp, G());
        }

        template<class Key, class GIST_TRAITS, class Allocator, class LESS, class key_policy>
        bool compare_key(const KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>& l, const KeyWithGist<Key, GIST_TRAITS, key_policy>& r, LESS comp)
        {
            typedef typename
                    boost::integral_constant<
                          bool,
                          KeyWrapper<Key, GIST_TRAITS, Allocator, key_policy>::has_gist && KeyWithGist<Key, GIST_TRAITS, key_policy>::has_gist
                   >::type G;

            return compare_key_helper(l, r, comp, G());
        }

        template<typename Allocator, typename Key, typename value_type, typename KeyWrapper, typename key_extractor, typename ValuePolicy>
        struct LeafNodeImpl : public Allocator
        {
            typedef KeyWrapper KeyHolder;

            LeafNodeImpl(const Allocator& a, const value_type& v)
                : Allocator(a)
                , _value(0)
                , _key(key_extractor()(v))
            {
                _value = this->allocate(1);
                this->construct(_value, v);
            }

            ~LeafNodeImpl() 
            {
                this->destroy(_value);
                this->deallocate(_value, 1);
            }

            value_type& value()             { return *_value; }
            const value_type& value() const { return *_value; }
            const KeyHolder& key() const    { return _key;    }

            value_type*       _value;
            const KeyHolder   _key;
        };

        template<typename Allocator, typename Key, typename value_type, typename KeyWrapper, typename key_extractor>
        struct LeafNodeImpl<Allocator, Key, value_type, KeyWrapper, key_extractor, boost::true_type>
        {
            typedef Key KeyHolder;
            LeafNodeImpl(Allocator, const value_type& v) : _value(v) { }

            const KeyHolder& key() const    { return key_extractor()(_value); }
            value_type& value()             { return _value; }
            const value_type& value() const { return _value; }

            value_type _value;
        };

        template<
              typename Key
            , typename ValueType
            , typename Allocator
            , typename comparator
            , typename key_extractor
            , int BN
            , class key_policy
            , class value_policy
            , class gist_traits
        >
        class stree
            : public Allocator
        {
        public:
            typedef typename Allocator::size_type size_type;
            typedef ValueType value_type;
        private:
            typedef stree<
                Key, value_type, Allocator,
                comparator, key_extractor, BN, 
                key_policy, value_policy, gist_traits
            > container_type;

            template<typename NODE>
            struct TBNode;
            struct BaseBNode;

            typedef typename Allocator::template rebind<BaseBNode>::other::pointer base_bnode_ptr;
            typedef typename Allocator::template rebind<Key>::other KeyAllocator;

            typedef KeyWrapper<Key, gist_traits, KeyAllocator, key_policy> KeyWrapperType;
            typedef KeyWithGist<Key, gist_traits, key_policy > KeyWithGistType;


            struct InnerNode : boost::noncopyable
            {
                typedef KeyWrapperType KeyHolder;
                InnerNode() {}
                explicit InnerNode(const KeyHolder& k) : _key(k), down(0) {}
                explicit InnerNode(const Key& k) : _key(k), down(0) {}
                ~InnerNode() { }

                KeyHolder     _key;
                base_bnode_ptr down;

                void set_key(const Key& k)          { _key = k; }
                void set_key(const KeyWithGistType& k)  { _key = k; }
                void set_key(const KeyHolder& k)    { _key = k; }
                const KeyHolder& key()  const       { return _key; }
            };

            struct LeafNode : public LeafNodeImpl<Allocator, Key, value_type, KeyWrapperType, key_extractor, value_policy>
            {
                typedef LeafNodeImpl<Allocator, Key, value_type, KeyWrapperType, key_extractor, value_policy> base;
                typedef typename base::KeyHolder KeyHolder;
                LeafNode(const Allocator& a, const value_type& v) : base(a, v) {}
            };

            inline void moveitems(InnerNode* dest, const InnerNode* src, size_t count)
            {
                 ::memmove(dest, src, sizeof(InnerNode)*count);
            }

            inline void moveitems(LeafNode* dest, const LeafNode* src, size_t count)
            {
                ::memmove(dest, src, sizeof(LeafNode)*count);
            }

            struct BaseBNode
            {
                typedef typename Allocator::template rebind<BaseBNode>::other BaseAlloc;
                typedef typename Allocator::template rebind<TBNode<InnerNode> >::other ParentAlloc;
                typedef typename ParentAlloc::pointer parent_ptr;
                typedef typename BaseAlloc::pointer base_ptr;
                typedef typename BaseAlloc::const_pointer const_base_ptr;

                BaseBNode(bool l)
                    : leaf(l)
                    , count(0)
                    , parent(0)
                    , right(0)
                    , left(0)
                {}

                bool full() const       { BOOST_ASSERT(count <= BN); return count == BN; }
                int size() const        { return count; }
                bool empty() const      { return count == 0; }

                bool is_leaf() const    { return leaf; }

                const bool    leaf;
                short         count;

                parent_ptr    parent;
                base_ptr      right;
                base_ptr      left;
            };

            template<typename NODE>
            struct TBNode : public BaseBNode
            {
                typedef NODE N;
                typedef TBNode<N> node_type;
                typedef typename BaseBNode::base_ptr base_ptr;
                typedef typename BaseBNode::const_base_ptr const_base_ptr;

                typedef typename Allocator::template rebind<node_type>::other Alloc;
                typedef typename Alloc::pointer ptr;
                typedef typename Alloc::const_pointer cptr;

                //N _buffer[BN];
                char* _buffer[sizeof(N)*BN];

                // Helper functions - to simplify coding
                N* get_node(int n)             { return ((N*)_buffer)+n; }
                const N* get_node(int n) const { return ((const N*)_buffer)+n; }

                N& operator[](int n)                { return *get_node(n); }
                const N& operator[](int n) const    { return *get_node(n); }

                TBNode()
                    : BaseBNode(boost::is_same<N, LeafNode>::value)
                {}

#define FIND_UNLOOP_N 8

                template<typename K>
                int find(const K& k) const
                {
                    int low = 0;
                    int high = this->count;
                    while (high > low + FIND_UNLOOP_N)
                    {
                        int mid = (high + low)/2;
                        if (less(this->get_node(mid)->key(), k))
                            low = mid + 1;
                        else
                            high = mid;
                    }
                    for (;low < high; ++low)
                        if (!less(this->get_node(low)->key(), k))
                            return low;

                    return low;
                }

            public:
                cptr next() const { return cast(this->right); }
                ptr next()        { return cast(this->right); }
                cptr prev() const { return cast(this->left); }
                ptr prev()        { return cast(this->left); }

            private:
                cptr cast(const_base_ptr p) const
                {
                    return static_cast<cptr>(p);
                }

                ptr cast(base_ptr p)
                {
                    return static_cast<ptr>(p);
                }
            };

            typedef TBNode<LeafNode> LeafBNode;
            typedef TBNode<InnerNode> InnerBNode;

            typedef typename Allocator::template rebind<BaseBNode>::other::pointer pointer;
            typedef typename LeafBNode::ptr leaf_pointer;
            typedef typename LeafBNode::cptr const_leaf_pointer;
            typedef typename InnerBNode::ptr inner_node_pointer;

            struct DummyNode : public BaseBNode
            {
                DummyNode() 
                    : BaseBNode(true)
                    , revision(0)
                {}

                int revision;
            };

        public: // XXX
            // Iterators
            template <class V>
            class iter
                : public boost::iterator_facade<
                       iter<V>
                       , V
                       , boost::bidirectional_traversal_tag
                       , V&
                       , size_type
                  >
            {

            private:
                typedef boost::iterator_facade<
                    iter<V>,V,
                    boost::bidirectional_traversal_tag, V&, size_type
                > super_t;

                struct enabler {};

                friend class boost::iterator_core_access;
                template<class other_value_type>
                friend class iter;
                friend class stree<Key,ValueType,Allocator,comparator,key_extractor,BN, key_policy, value_policy, gist_traits>;
            private:

                iter(const DummyNode* d, leaf_pointer n, int i)
                    : _bnode(n)
                    , _idx(i)
                    , _dummy(d)
                    , _revision(_dummy->revision)
                {
                    BOOST_ASSERT(_idx == 0 || _idx < _bnode->count);
                }

                V& dereference() const
                {
                    if (_dummy->revision != _revision)
                        throw std::invalid_argument("stale iterator");
                    return (*_bnode)[_idx].value();
                }

                template<class other_value_type>
                bool equal(iter<other_value_type> const& other) const
                {
                    return (this->_bnode == other._bnode && this->_idx == other._idx);
                }

                void increment()
                {
                    ++_idx;
                    if (_idx >= _bnode->count)
                    {
                        _bnode = _bnode->next();
                        _idx = 0;
                    }
                }

                void decrement()
                {
                    if (_idx > 0)
                        --_idx;
                    else
                    {
                        _bnode = _bnode->prev();
                        BOOST_ASSERT(_bnode);
                        _idx = (static_cast<base_bnode_ptr>(_bnode) == _dummy) 
                                      ? 0 
                                      : _bnode->count - 1;
                    }
                }
            private:

                template<class Other> 
                struct if_convertible 
                {
                    typedef typename boost::enable_if<boost::is_convertible<Other, V> >::type type;
                };
            public:
                iter() 
                    : _bnode(0)
                    , _idx(0)
                    , _dummy(0)
                    , _revision(0)
                {
                }

                template <class Other>
                iter(const iter<Other>& other , typename if_convertible<Other>::type* e = 0)
                    : _bnode(other._bnode)
                    , _idx(other._idx)
                    , _dummy(other._dummy)
                    , _revision(other._revision)
                {}
            private:
                leaf_pointer    _bnode;
                int                _idx;
                const DummyNode* _dummy;
                int                _revision;
            };

        public:
            typedef iter<value_type> iterator;
            typedef iter<const value_type> const_iterator;
            typedef typename boost::reverse_iterator<iterator> reverse_iterator;
            typedef typename boost::reverse_iterator<const_iterator> const_reverse_iterator;

        private:
            inner_node_pointer _head;
            leaf_pointer       _first;
            size_type          _size;

            DummyNode          _dummy;

            const KeyWrapperType& last(inner_node_pointer p) const
            {
                BOOST_ASSERT(p->count > 0);
                return p->get_node(p->count - 1)->key();
            }
            const typename LeafNode::KeyHolder& last(leaf_pointer p) const
            {
                BOOST_ASSERT(p->count > 0);
                return p->get_node(p->count - 1)->key();
            }

            template<class N>
            void erase_item(TBNode<N>* p, int i)
            {
                BOOST_ASSERT(i < p->count);
                typedef typename Allocator::template rebind<N>::other Alloc;
                typename Alloc::pointer node = p->get_node(i);
                Alloc(*this).destroy(node);
                moveitems(node, node + 1, p->size() - i - 1);
                p->count --;
            }

            void add_node(InnerBNode* p, int i, const KeyWithGistType& v)
            {
                typedef typename Allocator::template rebind<InnerNode>::other Alloc;
                typename Alloc::pointer n = p->get_node(i);
                moveitems(n+1, n, (p->count - i));
                p->count ++;
                new(n) InnerNode(v);
                //Alloc(*this).construct(n, v);
            }

            void add_node(InnerBNode* p, int i, const Key& v)
            {
                typedef typename Allocator::template rebind<InnerNode>::other Alloc;
                typename Alloc::pointer n = p->get_node(i);
                moveitems(n+1, n, (p->count - i));
                p->count ++;
                new(n) InnerNode(v);
                //Alloc(*this).construct(n, v);
            }

            void add_node(LeafBNode* p, int i, const value_type& v)
            {
                typedef typename Allocator::template rebind<LeafNode>::other Alloc;
                typename Alloc::pointer n = p->get_node(i);
                if (p->count - i > 0)
                    moveitems(n+1, n, (p->count - i));
                p->count ++;
                new(n) LeafNode(*this, v);
                //Alloc(*this).construct(n, v);
            }

        private:
            //////////////////////////////////////////////////////////////////////////
            //
            //                                Validate
            //
            //////////////////////////////////////////////////////////////////////////
            template<typename B>
            void check(B b) const
            {
                if (!b)
                    throw std::runtime_error("stree bug");
            }
            template<typename PTR>
            void validate_base(PTR p) const
            {
                check(p->size() >= 0);
                check(p->size() <= BN);
                if (p->parent)
                {
                    inner_node_pointer dad = p->parent;
                    check(dad->size() <= BN);
                    check(dad->size() > 0);
                    if (p->count > 0)
                    {
                        int idx = dad->find(this->last(p));
                        check(idx < dad->count);
                        pointer d = (*dad)[idx].down;
                        check(d == p);
                    }
                }
                for (int i = 1; i < p->size(); ++i)
                {
                    check(!less(p->get_node(i)->key(), p->get_node(i-1)->key()));
                    check(less(p->get_node(i-1)->key(), p->get_node(i)->key()));
                }
            }

            void validate(inner_node_pointer p) const
            {
                validate_base(p);
                check(p->size() > 0);
                for (int i = 0; i < p->size(); ++i)
                {
                    check(p->get_node(i)->down);
                    int child_size = p->get_node(i)->down->size();
                    if (child_size > 0)
                    {
                        const KeyWrapperType& k = p->get_node(i)->key();
                        if (p->get_node(i)->down->is_leaf())
                        {
                            leaf_pointer r = static_cast<leaf_pointer>(p->get_node(i)->down);
                            check(!less(k, r->get_node(child_size - 1)->key()));
                            check(r->parent == p);
                        }
                        else
                        {
                            inner_node_pointer r = static_cast<inner_node_pointer>(p->get_node(i)->down);
                            check(!less(k, r->get_node(child_size - 1)->key()));
                            check(r->parent == p);
                        }
                    }
                    else
                    {
                        check(p->get_node(i)->down->is_leaf());
                    }
                }
            }

            void validate(leaf_pointer p) const
            {
                validate_base(p);
                check(p->parent);
            }

            void validate(iterator it) const
            {
                check(it._idx >= 0);
                check(it._idx < BN);
                if (it._dummy)
                {
                    if (it != end())
                    {
                        check(it._revision == _dummy.revision);
                        check(it._idx < it._bnode->size());
                        validate(it._bnode);
                    }
                }
            }
#ifdef _DEBUG
#    define VALIDATE(x) this->validate(x)
#else
#    define VALIDATE(x) do {} while(0)
#endif

        private:

            //////////////////////////////////////////////////////////////////////////
            //
            //                        Helper functions
            //
            //////////////////////////////////////////////////////////////////////////

            // XXX as ugly as it can get...
            leaf_pointer dummy_node() const
            {
                typedef typename Allocator::template rebind<DummyNode>::other Alloc;
                typedef typename Allocator::template rebind<BaseBNode>::other BaseAlloc;

                BOOST_STATIC_ASSERT(sizeof(leaf_pointer) == sizeof(typename Alloc::pointer));
                BOOST_STATIC_ASSERT(sizeof(leaf_pointer) == sizeof(typename BaseAlloc::pointer));

                typename BaseAlloc::pointer rr = const_cast<typename Alloc::pointer>(Alloc(*this).address(_dummy));
                return static_cast<leaf_pointer>(rr);
            }

            leaf_pointer last_bnode() const
            {
                BOOST_ASSERT(!empty());
                return dummy_node()->prev();
            }

            const KeyWrapperType& max_key() const
            {
                return (*_head)[0].key();
            }

            void fix_parent(inner_node_pointer n, int from, int to)
            {
                for (int i = from; i < to; ++i)
                    n->get_node(i)->down->parent = n;
            }
            void fix_parent(leaf_pointer, int, int) {}


            template<typename PTR>
            PTR do_merge(PTR p)
            {
                VALIDATE(p);

                PTR n = p->next();
                VALIDATE(n);

                moveitems(p->get_node(p->count), n->get_node(0), n->size());

                // fix parent for merged nodes
                int new_count = p->count + n->count;
                this->fix_parent(p, p->count, new_count);

                p->count = new_count;
                p->right = n->right;
                if (p->right)
                    p->right->left = p;

                return n;
            }

            template<typename PTR>
            void do_redistribute(PTR p)
            {
                PTR r = p->next();
                int num_nodes = p->count + r->count;

                VALIDATE(p);
                VALIDATE(r);

                int new_size = num_nodes/2;
                if (p->count > new_size)    // move nodes from this node to right node
                {
                    int to_move = p->count - new_size;

                    moveitems(r->get_node(to_move), r->get_node(0), r->count);
                    moveitems(r->get_node(0), p->get_node(new_size), to_move);

                    // fix parent
                    this->fix_parent(r, 0, to_move);
                }
                else // move nodes from right to this
                {
                    int to_move = new_size - p->count;
                    moveitems(p->get_node(p->count), r->get_node(0), to_move);
                    moveitems(r->get_node(0), r->get_node(to_move), (r->count-to_move));

                    // fix parent
                    this->fix_parent(p, p->count, new_size);
                }

                p->count = new_size;
                r->count = num_nodes - p->count;

            }

            template<typename PTR>
            PTR do_split(PTR p, PTR n)
            {
                moveitems(n->get_node(0), p->get_node(p->count/2), ((p->count+1)/2));

                n->count = (p->count+1)/2;
                p->count /= 2;

                // fix parent for split nodes
                this->fix_parent(n, 0, n->count);

                n->right = p->right;
                n->left = p;
                p->right = n;

                if (n->right)
                    n->right->left = n;

                n->parent = p->parent;

                return n;
            }

        protected:

            // Helpers
            template<class T, class Q>
            static bool equal(const T& l, const Q& r)
            {
                return !less(l, r) && !less(r, l);
            }

            static bool less(const Key& l, const Key& r)                 { return compare_key(l, r, comparator()); }
            static bool less(const KeyWrapperType& l, const KeyWrapperType& r)   { return compare_key(l, r, comparator()); }
            static bool less(const KeyWithGistType& l, const KeyWithGistType& r) { return compare_key(l, r, comparator()); }
            static bool less(const KeyWithGistType& l, const KeyWrapperType& r)  { return compare_key(l, r, comparator()); }
            static bool less(const KeyWrapperType& l, const KeyWithGistType& r)  { return compare_key(l, r, comparator()); }

        private:
            leaf_pointer new_leaf()
            {
                typedef typename LeafBNode::Alloc Alloc;
                Alloc al(*this);
                typename Alloc::pointer p = al.allocate(1);
                al.construct(p, LeafBNode());
                return p;
            }

            inner_node_pointer new_inner_node()
            {
                typedef typename InnerBNode::Alloc Alloc;
                Alloc al(*this);
                typename Alloc::pointer p = al.allocate(1);
                al.construct(p, InnerBNode());
                return p;
            }

            void delete_leaf(leaf_pointer p)
            {
                typedef typename LeafBNode::Alloc Alloc;
                Alloc al(*this);
                al.destroy(p);
                al.deallocate(p, 1);
            }

            void delete_node(inner_node_pointer p)
            {
                typedef typename InnerBNode::Alloc Alloc;
                Alloc al(*this);
                al.destroy(p);
                al.deallocate(p, 1);
            }

            void init(const KeyWithGistType& k)
            {
                leaf_pointer leaf = new_leaf();
                _head = new_inner_node();

                typedef typename LeafBNode::Alloc Alloc;
                Alloc al(*this);

                leaf->left = dummy_node();
                leaf->right = dummy_node();
                leaf->parent = _head;

                _dummy.parent = 0;
                _dummy.left = _dummy.right = leaf;
                _dummy.count = 0;

                add_node(_head, 0, extract_key(k));

                _first = leaf;

                InnerNode& hr = (*_head)[0];
                hr.down = leaf;
                _size = 0;
            }

            void done()
            {
                this->clear();
            }

            // Merge node with the next
            void merge_nodes(inner_node_pointer parent, int idx)
            {
                pointer p = (*parent)[idx].down;

                VALIDATE(parent);
                if (p->is_leaf())
                {
                    leaf_pointer r = static_cast<leaf_pointer>(p);
                    leaf_pointer d = do_merge(r);
                    delete_leaf(d);
                }
                else
                {
                    inner_node_pointer r = static_cast<inner_node_pointer>(p);
                    inner_node_pointer d = do_merge(r);
                    delete_node(d);
                }

                erase_item(parent, idx);
                (*parent)[idx].down = p;
                VALIDATE(parent);
            }

            // Redistribute nodes between 2 nodes
            void redistribute_nodes(inner_node_pointer parent, int idx)
            {
                pointer p = (*parent)[idx].down;

                // we need to move around a few nodes
                if (p->is_leaf())
                {
                    leaf_pointer r = static_cast<leaf_pointer>(p);
                    do_redistribute(r);
                    (*parent)[idx].set_key(this->last(r));
                }
                else
                {
                    inner_node_pointer r = static_cast<inner_node_pointer>(p);
                    do_redistribute(r);
                    (*parent)[idx].set_key(this->last(r));
                }
                VALIDATE(parent);
            }

            void do_combine(inner_node_pointer parent, int idx)
            {
                BOOST_ASSERT(idx < parent->size() - 1);
                int node_count1 = (*parent)[idx].down->size();
                int node_count2 = (*parent)[idx+1].down->size();

                if (node_count1 + node_count2 < BN)
                    merge_nodes(parent, idx);
                else
                    redistribute_nodes(parent, idx);
                VALIDATE(parent);

            }
            int combine_nodes(inner_node_pointer parent, int idx)
            {
                BOOST_ASSERT(parent->size() > 1);
                int node_idx;
                int next_sz = BN + 1;
                int prev_sz = BN + 1;

                if (idx + 1 < parent->size())
                    next_sz = (*parent)[idx+1].down->size();
                if (idx > 0)
                    prev_sz = (*parent)[idx-1].down->size();

                node_idx = (next_sz < prev_sz) 
                                    ? idx
                                    : idx - 1;
                BOOST_ASSERT(node_idx >= 0 && (node_idx < parent->size() - 1));
                do_combine(parent, node_idx);

                VALIDATE(parent);

                return node_idx;
            }

            const_iterator begin_helper() const
            {
                if (_first && _first->size() > 0)
                    return const_iterator(&_dummy, _first, 0);
                return end_helper();
            }

            const_iterator end_helper() const
            {
                return const_iterator(&_dummy, dummy_node(), 0);
            }

        public:

            std::pair<leaf_pointer, int> find_leaf(const KeyWithGistType& k) const
            {               
                if (empty() || !less(max_key(), k))
                {
                    pointer x = _head;
                    while (x)
                    {
                        // we found the gap to drop to
                        if (x->is_leaf())
                        {
                            // We're at the bottom
                            leaf_pointer leaf = static_cast<leaf_pointer>(x);
                            VALIDATE(leaf);

                            int i = leaf->find(k);
                            if (i < leaf->size())
                                return std::make_pair(leaf, i);
                            break;
                        }

                        inner_node_pointer node = static_cast<inner_node_pointer>(x);
                        int i = node->find(k);
                        BOOST_ASSERT(i < node->size());
                        VALIDATE(node);

                        pointer down = (*node)[i].down;
                        x = down;
                    }
                }
                return std::make_pair((leaf_pointer)0,0);
            }

            const_iterator find_helper(const KeyWithGistType& k) const
            {
                std::pair<leaf_pointer, int> pos = find_leaf(k);
                if (pos.first)
                {
                    if (equal((*pos.first)[pos.second].key(), k))
                        return iterator(&_dummy, pos.first, pos.second);
                    else
                        return end();
                }
                return end();
            }

            const_iterator lower_bound_helper(const KeyWithGistType& k) const
            {
                std::pair<leaf_pointer, int> pos = find_leaf(k);
                if (pos.first)
                {
                    VALIDATE(pos.first);
                    return iterator(&_dummy, pos.first, pos.second);
                } else
                    return end();
            }

            const_iterator upper_bound_helper(const KeyWithGistType& k) const
            {
                std::pair<leaf_pointer, int> pos = find_leaf(k);
                if (pos.first)
                {
                    VALIDATE(pos.first);

                    iterator it = iterator(&_dummy, pos.first, pos.second);
                    if (less(k, (*pos.first)[pos.second].key()))
                        return it;
                    else
                        return ++it;
                }
                else
                    return end();
            }

            std::pair<pointer, Key> split_node(inner_node_pointer parent, int node_idx, pointer node)
            {
                pointer next_node;
                Key l;
                BOOST_ASSERT((*parent)[node_idx].down == node);
                if (node->is_leaf())
                {
                    leaf_pointer leaf = static_cast<leaf_pointer>(node);
                    next_node = do_split(leaf, new_leaf());
                    l = extract_key(last(leaf));
                }
                else
                {
                    inner_node_pointer inode = static_cast<inner_node_pointer>(node);
                    next_node = do_split(inode, new_inner_node());
                    l = extract_key(last(inode));
                }
                add_node(parent, node_idx, l);
                (*parent)[node_idx].down = node;
                (*parent)[node_idx+1].down = next_node;

                VALIDATE(parent);
                return std::make_pair(next_node, l);
            }

            void destroy_stree(pointer head)
            {
                int sz = head->size();
                if (head->is_leaf())
                {
                    leaf_pointer l = static_cast<leaf_pointer>(head);
                    for (int i = sz - 1; i >= 0; --i)
                        erase_item(l, i);
                    delete_leaf(l);
                }
                else
                {
                    inner_node_pointer node = static_cast<inner_node_pointer>(head);
                    for (int i = sz - 1; i >= 0; --i)
                    {
                        pointer d = (*node)[i].down;
                        destroy_stree(d);
                        erase_item(node, i);
                    }
                    delete_node(node);
                }
            }

        public:
           stree(Allocator al = Allocator())
                : Allocator(al)
                , _head(0)
                , _first(0)
                , _size(0)
            {
            }

            stree(const container_type& other)
                : Allocator(other)
                , _head(0)
                , _first(0)
                , _size(0)
            {
                for (const_iterator it = other.begin(); it != other.end(); ++it)
                    insert(*it);
            }

            ~stree()
            {
                done();
            }

            void swap(container_type& other)
            {
                std::swap(other._dummy.right, this->_dummy.right);
                std::swap(other._dummy.left, this->_dummy.left);

                if (this->_dummy.right)
                    this->_dummy.right->left = dummy_node();
                if (this->_dummy.left)
                    this->_dummy.left->right = dummy_node();
                if (other._dummy.right)
                    other._dummy.right->left = other.dummy_node();
                if (other._dummy.left)
                    other._dummy.left->right = other.dummy_node();

                std::swap(other._head, this->_head);
                std::swap(other._first, this->_first);
                std::swap(other._size, this->_size);
            }

            iterator begin()                      { return iterator(begin_helper()); }
            iterator end()                        { return iterator(end_helper()); }

            const_iterator begin() const          { return begin_helper(); }
            const_iterator end() const            { return end_helper(); }

            reverse_iterator rbegin()             { return reverse_iterator(end()); }
            reverse_iterator rend()               { return reverse_iterator(begin()); }
            const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
            const_reverse_iterator rend() const   { return const_reverse_iterator(begin()); }

            std::pair<iterator, bool> insert(const value_type& v)
            {
                KeyWithGistType search_key(key_extractor()(v));
                if (!_head)
                    init(search_key);

                _dummy.revision++;

                pointer x = _head;
                iterator it = end();
                bool inserted = false;

                while (x)
                {
                    // we found the gap to drop to
                    if (x->is_leaf())
                    {
                        BOOST_ASSERT(!x->full());
                        // We're at the bottom
                        leaf_pointer p = static_cast<leaf_pointer>(x);
                        VALIDATE(p);
                        int i = p->find(search_key);

                        if (i < p->size() && equal((*p)[i].key(), search_key))
                        {
                            // Found item with same key
                            it = iterator(&_dummy, p, i);
                            inserted = false;
                            break;
                        }

                        add_node(p, i, v);

                        it = iterator(&_dummy, p, i);
                        inserted = true;
                        VALIDATE(p);
                        break;
                    }

                    // else - internal node
                    inner_node_pointer p = static_cast<inner_node_pointer>(x);
                    VALIDATE(p);

                    int i = p->size() - 1;
                    if (less((*p)[i].key(), search_key))
                        (*p)[i].set_key(search_key);
                    else
                        i = p->find(search_key);

                    pointer down = (*p)[i].down;
                    if (down->full())
                    {
                        std::pair<pointer, Key> n = split_node(p, i, down);

                        // choose where to go next
                        if (less(n.second, search_key))
                            down = n.first;

                        VALIDATE(p);
                    }

                    x = down;
                }

                if (_head->size() > 1)
                {
                    // Create a new level
                    inner_node_pointer old_head = _head;
                    inner_node_pointer new_head = new_inner_node();
                    add_node(new_head, 0, extract_key((*old_head)[old_head->size()-1].key()));
                    (*new_head)[0].down = old_head;

                    new_head->right = new_head->left = 0;

                    old_head->parent = new_head;

                    _head = new_head;
                    VALIDATE(_head);
                    VALIDATE(old_head);
                }

                if (inserted)
                    _size ++;
                return 
                    std::make_pair(it, inserted);
            }

            iterator insert(iterator _Where, const value_type& _Val)
            {
                VALIDATE(_Where);
                KeyWithGistType search_key(key_extractor()(_Val));
                if (!_head)
                    init(search_key);

                VALIDATE(_head);

                if (!this->empty() && _Where != end())
                {
                    if (!_Where._bnode->full())
                    {
                        KeyWithGistType cur(key_extractor()(*_Where));

                        // The hint should point into the element that will _precede_ the new element
                        bool b1 = less(cur, search_key);
                        bool b2 = less(search_key, max_key());
                        if ( b1 && b2 )
                        {
                            leaf_pointer n = _Where._bnode;
                            VALIDATE(n);
                            int pos = _Where._idx + 1;
                            ++_Where;
                            if (pos < n->size() && (_Where == end() || less(search_key, key_extractor()(*_Where))))
                            {
                                _dummy.revision++;

                                add_node(n, pos, _Val);
                                _size++;
                                VALIDATE(n);
                                return iterator(&_dummy, n, pos);
                            }
                        }
                    }
                }

                // Revert to the standard insert
                boost::tie(_Where, boost::tuples::ignore) = insert(_Val);
                return _Where;
            }

            iterator erase(iterator it)
            {

                VALIDATE(it);
                BOOST_ASSERT(it != end());

                _dummy.revision++;
                // We start at the _bottom_
                leaf_pointer n = it._bnode;
                int pos = it._idx;

                Key key = extract_key((*n)[pos].key());
                KeyWithGistType k(key);
                VALIDATE(n);

                erase_item(n, pos);
                _size --;
                VALIDATE(n);

                inner_node_pointer p = n->parent;
                if (n->size() < BN/2 && p != _head )
                {
                    BOOST_ASSERT(p->size() > 1);
                    // Need to fix the tree
                    // We do the first iteration of the while loop first
                    // so we can fix the iterator

                    // Search for element in its parent's array.
                    int i = p->find(k);
                    BOOST_ASSERT(i < p->size());
                    BOOST_ASSERT((*p)[i].down == n);

                    int prev_count = 0;
                    if (i > 0)
                        prev_count = (*p)[i - 1].down->size();
                    int node_idx = combine_nodes(p, i);
                    BOOST_ASSERT(node_idx <= i && (node_idx + 1 >= i));
                    BOOST_ASSERT(node_idx >= 0);
                    if (node_idx < i)
                        pos += prev_count;

                    n = static_cast<leaf_pointer>((*p)[node_idx].down);

                    VALIDATE(p);
                    VALIDATE(n);
                    // Now go up the stree to fix whatever needs fixing

                    pointer d = p;
                    p = p->parent;
                    while(p != _head && d->size() < BN/2)
                    {
                        VALIDATE(p);

                        // Search for 'd' in its parent's array.
                        int i = p->find(k);
                        BOOST_ASSERT(i < p->size());

                        combine_nodes(p, i);
                        VALIDATE(p);

                        d = p;
                        p = p->parent;
                    }

                    if (p == _head && d->size() == 1)
                    {
                        BOOST_ASSERT(!d->is_leaf());
                        // we can reduce height
                        delete_node(_head);
                        _head = static_cast<inner_node_pointer>(d);
                        _head->parent = 0;
                    }
                }    

                // fix pos
                if (pos >= n->size() && n != dummy_node())
                {
                    pos -= n->size();
                    n = n->next();

                    if (pos >= n->size() && n != dummy_node())
                    {
                        BOOST_ASSERT(pos == n->size());
                        pos -= n->size();
                        n = n->next();

#ifdef _DEBUG
                        BOOST_ASSERT(n == dummy_node() || !less((*n)[pos].key(), k));
                        if (pos > 0)
                            BOOST_ASSERT(n != dummy_node() && less((*n)[pos-1].key(), k));
                        else
                        {
                            leaf_pointer pr = n->prev();
                            BOOST_ASSERT(pr== dummy_node() || less((*pr)[pr->size() - 1].key(), k));
                        }
#endif
                    }
                }

                return iterator(&_dummy, n, pos);
            }

            // Very inefficient...
            iterator erase(iterator from, iterator to)
            {
                VALIDATE(from);
                VALIDATE(to);
                while (from != to)
                    from = erase(from);
                return to;
            }

            size_type erase(const Key& k)
            {
                KeyWithGistType key(k);
                if (less(max_key(), key))
                    return 0;

                _dummy.revision++;
                pointer x = _head;
                while (x)
                {
                    // reached bottom?
                    if (x->is_leaf())
                    {
                        leaf_pointer leaf = static_cast<leaf_pointer>(x);
                        VALIDATE(leaf);

                        int i = leaf->find(key);
                        BOOST_ASSERT(i >= 0 && i <= leaf->size());

                        if (i < leaf->size() && equal((*leaf)[i].key(), key))
                        {
                            erase_item(leaf, i);
                            _size --;
                            break;
                        }
                        else
                            return 0;
                    }

                    // Internal node
                    inner_node_pointer node = static_cast<inner_node_pointer>(x);
                    VALIDATE(node);
                    // Go over the current BNode
                    int i = node->find(key);

                    BOOST_ASSERT(i < node->size());

                    pointer down = (*node)[i].down;

                    // If the next node to visit has few elements, it may be
                    // possible to merge it with its pred/succ.
                    if (down->size() < BN/2)
                    {
                        if (node != _head)
                        {
                            i = combine_nodes(node, i);

                            // choose the down path
                            if (less((*node)[i].key(), key))
                                ++i;

                            BOOST_ASSERT(i < node->size());

                            down = (*node)[i].down;

                            VALIDATE(node);
                        }
                        else
                        {
                            // If 'down' was left with a single node, and its parent is _head,
                            // then we can make it the new _head, reducing the height by 1
                            // (but make sure we have at least head & bottom levels)
                            if (down->size() == 1 && !down->is_leaf())
                            {
                                delete_node(node);
                                _head = static_cast<inner_node_pointer>(down);
                                _head->parent = 0;
                                VALIDATE(_head);
                            }
                        }
                    }
                    x = down;
                }
                return 1;
            }

            iterator upper_bound(const Key& k)
            {
                return iterator(upper_bound_helper(k));
            }
            const_iterator upper_bound(const Key& k) const
            {
                return iterator(upper_bound_helper(k));
            }

            std::pair<iterator, iterator> equal_range(const Key& k)
            {
                return std::make_pair(lower_bound(k), upper_bound(k));
            }

            size_type count( const Key& key ) const
            {
                if (find(key) != end())
                    return 1;
                return 0;
            }

            const_iterator find(const Key& k) const
            {
                return find_helper(k);
            }

            iterator find(const Key& k)
            {
                KeyWithGistType key(k);
                return iterator(find_helper(key));
            }

            const_iterator lower_bound(const Key& k) const
            {
                KeyWithGistType key(k);
                return iterator(lower_bound_helper(key));
            }

            iterator lower_bound(const Key& k)
            {
                KeyWithGistType key(k);
                return iterator(lower_bound_helper(key));
            }

            size_type size() const
            {
                return _size;
            }

            bool empty() const
            {
                return _size == 0;
            }


            void clear()
            {
                if (_head)
                    destroy_stree(_head);

                _first = 0;
                _size = 0;
                _head = 0;
            }

            bool is_equal(const stree<Key, value_type, Allocator,comparator, 
                                        key_extractor, BN, key_policy, value_policy, gist_traits>& other) const
            {
                if (size() != other.size())
                    return false;
                const_iterator it1, it2;

                //std::find_if(
                //    boost::make_zip_iterator(boost::make_tuple(begin(), other.begin())),
                //    boost::make_zip_iterator(boost::make_tuple(end(), other.end())),
                //    zip_func()
                //    );

                for (it1 = begin(), it2 = other.begin(); it1 != end() && it2 != other.end(); ++it1, ++it2)
                    if (*it1 != *it2)
                        return false;
                return true;
            }

            bool operator==(const container_type& other) const { return is_equal(other); }
            bool operator!=(const container_type& other) const { return !operator==(other); }
        };
    }
}

#endif // __STI_STREE_H__
