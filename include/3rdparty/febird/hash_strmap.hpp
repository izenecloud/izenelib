#ifndef __febird_fast_hash_strmap2__
#define __febird_fast_hash_strmap2__

#include <string>
#include <ostream>

#include "hash_common.hpp"

// http://software.intel.com/en-us/articles/memcpy-performance
//  This link mentioned memcpy may be optimized by hardware, Whether aligned or unaligned:
//    hardware implementations which automatically switch into wide move modes
//    would be more attractive than software support
#define HSM_ENABLE_MY_memcpy

#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
    #define HSM_FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define HSM_FORCE_INLINE __forceinline
#else
    #define HSM_FORCE_INLINE inline
#endif

// Why name it SP_ALIGN? It is: String Pool/Pointer Align
//
// to disable align, define SP_ALIGN as 0
//    otherwise, it will guess and use the best alignment
#ifndef SP_ALIGN
    #if defined(__GNUC__) && ( \
          defined(__LP64__) && (__LP64__ == 1) || \
          defined(__amd64__) || defined(__amd64) || \
          defined(__x86_64__) || defined(__x86_64) || \
          defined(__ia64__) || defined(_IA64) || defined(__IA64__) ) || \
        defined(_MSC_VER) && ( defined(_WIN64) || defined(_M_X64) || defined(_M_IA64) ) || \
        defined(__INTEL_COMPILER) && ( \
          defined(__ia64) || defined(__itanium__) || \
          defined(__x86_64) || defined(__x86_64__) )
      #define SP_ALIGN 8
    #else
      #define SP_ALIGN 4
    #endif
#endif

#if defined(SP_ALIGN) && SP_ALIGN != 0
    #if SP_ALIGN == 4
    //	typedef uint32_t align_type;
        typedef unsigned align_type;
    #elif SP_ALIGN == 8
    //	typedef uint64_t align_type;
        typedef unsigned long long align_type;
    #else
        #error "SP_ALIGN defined but is not 0, 4 or 8"
    #endif
    #if SP_ALIGN == 4 && UINT_MAX != 0xFFFFFFFF
        #error "sizeof(unsigned) must be 4"
    #elif SP_ALIGN == 8 && ULLONG_MAX != 0xFFFFFFFFFFFFFFFFull
        #error "sizeof(unsigned long long) must be 8"
    #endif
    // on 64 bit system, make it to support larger strpool, up to 8*4G = 32G
    #define LOAD_OFFSET(x)  (size_t(x) * SP_ALIGN)
    #define SAVE_OFFSET(x)  LinkTp((x) / SP_ALIGN)
    #define IF_SP_ALIGN(Then, Else) Then
#else
    #define LOAD_OFFSET(x)  size_t(x)
    #define SAVE_OFFSET(x)  LinkTp(x)
    #define IF_SP_ALIGN(Then, Else) Else
#endif

// Fast String: shallow copy, simple, just has char* and length
// May be short name of: Febird String
struct fstring {
    const char* p;
    ptrdiff_t   n;

    fstring(const std::string& x) : p(x.data()), n(x.size()) {}
    fstring(const char* x)        : p(x), n(strlen(x)) {}
    fstring(const char* x, ptrdiff_t l) : p(x), n(l)   {}

    std::string   str() const { return std::string(p, n); }
    const char* c_str() const { return p; }
    size_t       size() const { return n; }

    // 3-way compare
    class prefix_compare3 {
        ptrdiff_t plen;
    public:
        int operator()(fstring x, fstring y) const {
            using namespace std;
            const ptrdiff_t lmin = min(x.n, y.n);
            const ptrdiff_t clen = min(lmin, plen);
            for (ptrdiff_t i = 0; i < clen; ++i)
                if (x.p[i] != y.p[i]) // char diff doesn't exceed INT_MAX
                    return (unsigned char)x.p[i] - (unsigned char)y.p[i];
            if (plen < lmin)
                return 0; // all prefix are same
            else
                return int(x.n - y.n); // length diff couldn't exceed INT_MAX
        }
        prefix_compare3(ptrdiff_t prelen) : plen(prelen) {}
    };

    // 3-way compare
    class compare3 {
    public:
        int operator()(fstring x, fstring y) const {
            using namespace std;
            int ret = memcmp(x.p, y.p, min(x.n, y.n));
            if (ret != 0)
                return ret;
            return int(x.n - y.n); // length diff couldn't exceed INT_MAX
        }
    };

    struct less_unalign {
        bool operator()(const fstring& x, const fstring& y) const {
            int ret = memcmp(x.p, y.p, x.n < y.n ? x.n : y.n);
            if (ret != 0)
                return ret < 0;
            else
                return x.n < y.n;
        }
    };
    struct hash_unalign {
        HSM_FORCE_INLINE
        size_t operator()(const fstring k) const {
            size_t h = 2134173 + k.n * 31;
            for (ptrdiff_t i = 0; i < k.n; ++i)
                h = h * 31 + k.p[i];
            return h;
        }
    };
    struct equal_unalign {
        bool operator()(const fstring x, const fstring y) const {
            return x.n == y.n && memcmp(x.p, y.p, x.n) == 0;
        }
    };

#if defined(SP_ALIGN) && SP_ALIGN != 0
    static size_t align_to(size_t x) {
        return (x + SP_ALIGN-1) & ~(intptr_t)(SP_ALIGN-1);
    }
    struct less_align {
        HSM_FORCE_INLINE
        bool operator()(const fstring& x, const fstring& y) const {
            ptrdiff_t n = x.n < y.n ? x.n : y.n;
            ptrdiff_t c = n - (SP_ALIGN-1);
            ptrdiff_t i = 0;
            for (; i < c; i += SP_ALIGN)
                if (*reinterpret_cast<const align_type*>(x.p + i) !=
                    *reinterpret_cast<const align_type*>(y.p + i))
                        break;
            for (; i < n; ++i)
                if (x.p[i] != y.p[i])
                    return (unsigned char)x.p[i] < (unsigned char)y.p[i];
            return x.n < y.n;
        }
    };
    struct Less {
        HSM_FORCE_INLINE
        bool operator()(const fstring& x, const fstring& y) const {
            if (((intptr_t(x.p) | intptr_t(y.p)) & (SP_ALIGN-1)) == 0)
                return less_align()(x, y);
            else
                return less_unalign()(x, y);
        }
    };
    struct hash_align {
        HSM_FORCE_INLINE
        size_t operator()(const fstring k) const {
            size_t h = 2134173 + k.n * 31;
            ptrdiff_t c = k.n - (SP_ALIGN-1);
            ptrdiff_t i = 0;
            for (; i < c; i += SP_ALIGN)
                h = h * 31 + *reinterpret_cast<const align_type*>(k.p + i);
        #if 1
            for (; i < k.n; ++i)
                h = h * 31 + k.p[i];
        #else // this loop unroll is slower on gcc
            switch (k.n % SP_ALIGN) {
            #if SP_ALIGN == 8
                case 7: h = h * 31 + k.p[i+7];
                case 6: h = h * 31 + k.p[i+6];
                case 5: h = h * 31 + k.p[i+5];
                case 4: h = h * 31 + k.p[i+4];
            #endif
                case 3: h = h * 31 + k.p[i+3];
                case 2: h = h * 31 + k.p[i+2];
                case 1: h = h * 31 + k.p[i+1];
                case 0: assert(0); break; // would not go here
            }
        #endif
            return h;
        }
    };

// make it easier to enable or disable X86 judgment
#if 1 && ( \
    defined(__i386__) || defined(__i386) || defined(_M_IX86) || \
    defined(__X86__) || defined(_X86_) || \
    defined(__THW_INTEL__) || defined(__I86__) || \
    defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) \
    )
    // don't care pointer align, overhead of x86 align fault is slightly enough
    typedef hash_align hash;
#else
    struct hash { // align or not align
        HSM_FORCE_INLINE
        size_t operator()(const fstring k) const {
            size_t h = 2134173 + k.n * 31;
            ptrdiff_t c = k.n - (SP_ALIGN-1);
            ptrdiff_t i = 0;
            if ((intptr_t(k.p) & (SP_ALIGN-1)) == 0) { // aligned
                for (; i < c; i += SP_ALIGN)
                    h = h * 31 + *reinterpret_cast<const align_type*>(k.p + i);
            }
            else { // not aligned
                for (; i < c; i += SP_ALIGN) {
                    align_type word;
                #if 0
                    word = (  (align_type)k.p[i+0]
                            | (align_type)k.p[i+1] <<  8
                            | (align_type)k.p[i+2] << 16
                            | (align_type)k.p[i+3] << 24
                    #if SP_ALIGN == 8
                            | (align_type)k.p[i+4] << 32
                            | (align_type)k.p[i+5] << 40
                            | (align_type)k.p[i+6] << 48
                            | (align_type)k.p[i+7] << 56
                    #endif
                        );
                #else
                    // compiler could generate unaligned load instruction
                    // for fixed size(SP_ALIGN) object, word may be a register
                    memcpy(&word, k.p + i, SP_ALIGN);
                #endif
                    h = h * 31 + word;
                }
            }
            for (; i < k.n; ++i)
                h = h * 31 + k.p[i];
            return h;
        }
    };
#endif // X86
    struct equal_align {
        HSM_FORCE_INLINE
        bool operator()(const fstring x, const fstring y) const {
            if (HSM_unlikely(x.n != y.n))
                return false;
            ptrdiff_t c = x.n - (SP_ALIGN-1);
            ptrdiff_t i = 0;
            for (; i < c; i += SP_ALIGN)
                if (*reinterpret_cast<const align_type*>(x.p + i) !=
                    *reinterpret_cast<const align_type*>(y.p + i))
                        return false;
            for (; i < x.n; ++i)
                if (x.p[i] != y.p[i])
                    return false;
            return true;
        }
    };
    struct equal { // align or not align
        HSM_FORCE_INLINE
        bool operator()(const fstring x, const fstring y) const {
            if (HSM_unlikely(x.n != y.n))
                return false;
            if (((intptr_t(x.p) | intptr_t(y.p)) & (SP_ALIGN-1)) == 0) {
                ptrdiff_t c = x.n - (SP_ALIGN-1);
                ptrdiff_t i = 0;
                for (; i < c; i += SP_ALIGN)
                    if (*reinterpret_cast<const align_type*>(x.p + i) !=
                        *reinterpret_cast<const align_type*>(y.p + i))
                            return false;
                for (; i < x.n; ++i)
                    if (x.p[i] != y.p[i])
                        return false;
                return true;
            } else
                return memcmp(x.p, y.p, x.n) == 0;
        }
    };
#else
    typedef less_unalign Less;
    typedef hash_unalign hash;
    typedef equal_unalign equal;
#endif // SP_ALIGN
};
std::ostream& operator<<(std::ostream& os, fstring s) {
    os.write(s.p, s.n);
    return os;
}

template<class DataIO>
void DataIO_saveObject(DataIO& dio, fstring s) {
    need_febird__var_size_t(dio); // keep, but make safe
    dio.ensureWrite(s.p, s.n);
}
bool operator==(fstring x, fstring y) { return  fstring::equal()(x, y); }
bool operator!=(fstring x, fstring y) { return !fstring::equal()(x, y); }

template<class Owner, class Key, class Val>
class hash_strmap_iterator_c {
protected:
    Owner* owner;
    size_t index;
public:
//	reference to reference is not allowed for std::pair in pre-C++0x
//	typedef std::pair<const Key, Val&> value_type;
    struct value_type {
        typedef const Key  first_type;
        typedef       Val second_type;
        Key     first;
        Val&   second;
        operator std::pair<const Key, Val>() {
          return std::pair<const Key, Val>(first, second);
        }
        value_type(const Key& key, Val& val) : first(key), second(val) {}
    };
    typedef ptrdiff_t difference_type;
    typedef    size_t       size_type;
    typedef const Key        key_type;
    typedef       Val     mapped_type;
    typedef value_type      reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    // OK for iterator::operator->()
    struct pointer : value_type {
        value_type* operator->() const { return const_cast<pointer*>(this); }
    };

    hash_strmap_iterator_c() : owner(NULL), index(0) {}
    hash_strmap_iterator_c(Owner* o, size_t i)	: owner(o), index(i) {}

    value_type operator*() const {
        assert(NULL != owner);
        assert(index < owner->end_i());
        return value_type(owner->key(index), owner->val(index));
    }
    pointer operator->() const {
        assert(NULL != owner);
        assert(index < owner->end_i());
        value_type kv(owner->key(index), owner->val(index));
        return static_cast<pointer&>(kv);
    }
    hash_strmap_iterator_c& operator++() {
        assert(index < owner->end_i());
        index = owner->next_i(index);
        return *this;
    }
    hash_strmap_iterator_c& operator--() {
        assert(index < owner->end_i());
        assert(index > 0);
        index = owner->prev_i(index);
        return *this;
    }
    hash_strmap_iterator_c operator++(int) {
        hash_strmap_iterator_c tmp(*this);
        ++*this;
        return tmp;
    }
    hash_strmap_iterator_c operator--(int) {
        hash_strmap_iterator_c tmp(*this);
        --*this;
        return tmp;
    }
    size_t get_index() const { return index; }
    Owner* get_owner() const { return owner; }

    friend bool operator==(hash_strmap_iterator_c x, hash_strmap_iterator_c y) {
        assert(x.owner == y.owner);
        return x.index == y.index;
    }
    friend bool operator!=(hash_strmap_iterator_c x, hash_strmap_iterator_c y) {
        assert(x.owner == y.owner);
        return x.index != y.index;
    }
};

template<class Owner, class Key, class Val>
class hash_strmap_iterator_v : public hash_strmap_iterator_c<Owner, Key, Val> {
public:
  hash_strmap_iterator_v() {}
  hash_strmap_iterator_v(Owner* o, size_t i)
    : hash_strmap_iterator_c<Owner, Key, Val>(o, i) {}
  operator hash_strmap_iterator_c<const Owner, Key, const Val>() const {
    return hash_strmap_iterator_c<const Owner, Key, const Val>(this->owner, this->index);
  }
};

// LinkTp is also offset-type
template<class LinkTp, class Value, class ValuePlace>
struct hash_strmap_node;
template<class LinkTp, class Value>
struct hash_strmap_node<LinkTp, Value, ValueOut> {
    LinkTp offset; // offset to string pool
    LinkTp link;   // link
};
template<class LinkTp, class Value>
struct hash_strmap_node<LinkTp, Value, ValueInline> {
    LinkTp offset; // offset to string pool
    LinkTp link;   // link
    Value  value;
};

//
// hash_strmap<> hset; // just a set, can be used as a string pool
//
template< class Value = ValueOut // ValueOut means empty value, just like a set
        , class HashFunc = fstring::IF_SP_ALIGN(hash_align, hash)
        , class KeyEqual = fstring::IF_SP_ALIGN(equal_align, equal)
        , class ValuePlace = typename boost::mpl::if_c<
                    !boost::is_empty<Value>::value && // ValueInline should be non-empty
                    sizeof(Value) % sizeof(intptr_t) == 0 && sizeof(Value) <= 32
                , ValueInline
                , ValueOut // If Value is empty, ValueOut will not use memory
            >::type
        , class CopyStrategy = FastCopy
        , class LinkTp = unsigned int // could be unsigned short for small map
        >
class hash_strmap
{
    struct ValueRaw {
        char block[sizeof(Value)];
    };
    typedef hash_strmap_node<LinkTp, Value, ValuePlace> Node;
    typedef size_t HashTp;

    static const LinkTp tail = dummy_bucket<LinkTp>::tail;
    static const LinkTp delmark = tail - 1;
    static const LinkTp maxlink = tail - 2; // all link must < maxlink
#if defined(SP_ALIGN) && SP_ALIGN != 0
    static const size_t maxoffset =	tail * (size_t)(sizeof(LinkTp) < sizeof(size_t) ? SP_ALIGN : 1);
#else
    static const size_t maxoffset =	tail;
#endif
    static const intptr_t dont_cache_hash = 8; // good if compiler aggressive optimize

    Node*   pNodes;
    LinkTp  nNodes;   // hard limit is maxlink, even in 64bit platform
    LinkTp  maxNodes; // hard limit is maxlink
    LinkTp  maxload;  // hard limit is maxlink, guard for calling rehash
    LinkTp  nDeleted;
    HashTp* pHash;   // array of cached hash value

    LinkTp* bucket;  // index to pNodes
    size_t  nBucket; // could be larger up to 4G-1 when size_t is 32 bits
                     // and more when size_t is 64 bits

    char*    strpool;
    size_t   lenpool;
    size_t   maxpool;  // hard limit is maxoffset
    size_t   freepool; // free pool size

    Value*   values;   // when value out

    double   load_factor;
    enum sort_type {
        en_unsorted,
        en_sort_by_key,
        en_sort_by_val
    } sort_flag;

    HashFunc hash;
    KeyEqual equal;

    void init() {
        pNodes = NULL;
        nNodes = 0;
        maxNodes = 0;
        maxload = 0;
        nDeleted = 0;
        pHash = NULL; // default enabled

        bucket = dummy_bucket<LinkTp>::b; // false start
        nBucket = 1;

        strpool = NULL;
        lenpool = 0;
        maxpool = 0;
        freepool = 0;

        values = NULL;

        load_factor = 0.3;
        sort_flag = en_unsorted;
    }

    Value& nth_value(size_t nth, ValueInline) const { return pNodes[nth].value; }
    Value& nth_value(size_t nth, ValueOut   ) const { return values[nth]; }

    Value& nth_value(Node* pn, Value*, size_t nth, ValueInline) const { return pn[nth].value; }
    Value& nth_value(Node*, Value* pv, size_t nth, ValueOut   ) const { return pv[nth]; }

    static size_t extralen(const char* ps, size_t End) {
        return IF_SP_ALIGN(ps[End-1], 0) + 1;
    }
    size_t extralen(size_t End) const { return extralen(strpool, End); }

    void move_cons(void* dest, Value* src, FastCopy) {
        *reinterpret_cast<ValueRaw*>(dest) = *reinterpret_cast<const ValueRaw*>(src);
    }
    void move_cons(void* dest, Value* src, SafeCopy) {
    #ifdef HSM_HAS_MOVE
        new(dest) Value(std::move(*src));
    #else
        move_cons_aux(dest, *src, boost::has_nothrow_constructor<Value>());
    #endif
    }
    typedef boost::mpl::true_  has_nothrow_cons_yes;
    typedef boost::mpl::false_ has_nothrow_cons_no;
    void move_cons_aux(void* dst_raw, Value& src, has_nothrow_cons_yes) {
        // assume nothrow cons is fast enough, such as stl containers
        new(dst_raw)Value();
        std::swap(*dst_raw, src);
        src.~Value();
    }
    void move_cons_aux(void* dst_raw, Value& src, has_nothrow_cons_no) {
        new(dst_raw)Value(src);
        src.~Value();
    }
    void move_value(Node*, Node*, Value* vdest, Value* vsrc, ValueOut) {
        move_cons(vdest, vsrc, CopyStrategy());
    }
    void move_value(Node* dest, Node* src, Value*, Value*, ValueInline) {
        move_cons(&dest->value, &src->value, CopyStrategy());
    }

    size_t allnodes_size(size_t n) const {
        // guard offset used for compute strlen of pNodes[nNodes-1]
        // guard link   used for speedup next_i
        size_t s = sizeof(Node) * n + sizeof(pNodes->offset) + sizeof(pNodes->link);
        return (s + 15) & (size_t)ptrdiff_t(~15); // align to 16
    }

    void reserve_nodes_impl(size_t cap, SafeCopy, ValueInline) {
        Node* pn = (Node*)malloc(allnodes_size(cap));
        if (NULL == pn) throw std::bad_alloc();
        if (pNodes) {
            Node* oldpn = pNodes;
            for (size_t i = 0, n = nNodes; i < n; ++i) {
            #ifdef HSM_HAS_MOVE
                new(pn+i) Node(std::move(oldpn[i])); // no throw
            #else
                // if it throw any exception below, the system is assumed unrecoverable
                // so don't care exception safe for below code
                new(pn+i) Node(oldpn[i]);
                oldpn[i].value.~Value();
            #endif
            }
            free(oldpn);
        }
        pn[nNodes].offset = SAVE_OFFSET(lenpool);
        pn[nNodes].link = tail; // guard for next_i
        pNodes = pn;
    }
    void reserve_nodes_impl(size_t cap, SafeCopy, ValueOut) {
        Node* pn = (Node*)realloc(pNodes, allnodes_size(cap));
        if (NULL == pn) throw std::bad_alloc();
        if (!is_value_empty) {
            Value* pv = (Value*)malloc(sizeof(Value) * cap);
            if (NULL == pv) throw std::bad_alloc();
            if (values) {
                Value* oldpv = values;
                for (size_t i = 0, n = nNodes; i < n; ++i) {
                #ifdef HSM_HAS_MOVE
                    new(pv+i) Value(std::move(oldpv[i]));
                #else
                    // if it throw any exception below, the system is assumed unrecoverable
                    // so don't care exception safe for below code
                    new(pv+i) Value(oldpv[i]);
                    oldpv[i].~Value();
                #endif
                }
                free(oldpv);
            }
            values = pv;
        }
        if (NULL == pNodes) {
            pn[0].offset = 0;
            pn[0].link = tail; // guard for next_i
        }
        pNodes = pn;
    }
    // This is the normal case:
    void reserve_nodes_impl(size_t cap, FastCopy, ValueInline) {
        Node* pn = (Node*)realloc(pNodes, allnodes_size(cap));
        if (NULL == pn) throw std::bad_alloc();
        if (NULL == pNodes) {
            pn[0].offset = 0;
            pn[0].link = tail; // guard for next_i
        }
        pNodes = pn;
    }
    void reserve_nodes_impl(size_t cap, FastCopy, ValueOut) {
        Node* pn = (Node*)realloc(pNodes, allnodes_size(cap));
        if (NULL == pn) throw std::bad_alloc();
        if (!is_value_empty) {
            Value* pv = (Value*)realloc(values, sizeof(Value) * cap);
            if (NULL == pv) {
                pNodes = pn; // maxNodes unchanged
                throw std::bad_alloc();
            }
            values = pv;
        }
        if (NULL == pNodes) {
            pn[0].offset = 0;
            pn[0].link = tail; // guard for next_i
        }
        pNodes = pn;
    }

    template<bool WithFill>
    void relink_impl() {
        assert(0 == nDeleted);
        LinkTp* pb = bucket;
        Node*  pn = pNodes;
        size_t nb = nBucket;
        // (LinkTp)tail is just used for coerce tail be passed by value
        // otherwise, tail is passed by reference, this will case g++ link error
        std::fill_n(pb, nb, (LinkTp)tail);
        if (intptr_t(pHash) == dont_cache_hash) {
            for (size_t j = 0, n = nNodes; j < n; ++j) {
                size_t ib = hash(key(j)) % nb;
                pn[j].link = pb[ib];
                pb[ib] = LinkTp(j);
            }
        }
        else {
            HashTp* ph = pHash;
            for (size_t j = 0, n = nNodes; j < n; ++j) {
                size_t ib = (WithFill ? (ph[j] = hash(key(j))) : ph[j]) % nb;
                pn[j].link = pb[ib];
                pb[ib] = LinkTp(j);
            }
        }
    }

    void relink() { relink_impl<false>(); }
    void relink_fill() { relink_impl<true>(); }

    void destroy() {
        if (pNodes) {
            if (!boost::has_trivial_destructor<Value>::value) {
                for (size_t i = nNodes; i > 0; --i)
                    if (pNodes[i-1].link != delmark)
                        nth_value(i-1, ValuePlace()).~Value();
            }
            free(pNodes);
        }
        if (pHash && intptr_t(pHash) != dont_cache_hash)
            free(pHash);
        if (bucket && dummy_bucket<LinkTp>::b != bucket)
            free(bucket);
        if (strpool)
            free(strpool);
        if (is_value_out && !is_value_empty && values)
            free(values);
    }

public:
    typedef hash_strmap_iterator_v<hash_strmap, fstring, Value> iterator;
    typedef hash_strmap_iterator_c<const hash_strmap, fstring, const Value> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef typename iterator::difference_type difference_type;
    typedef typename iterator::size_type  size_type;
    typedef typename iterator::value_type value_type;
    typedef typename iterator::reference  reference;

    typedef const fstring key_type;
    typedef Value mapped_type;

    static const bool is_value_out = boost::is_same<ValuePlace, ValueOut>::value;
    static const bool is_value_empty = boost::is_empty<Value>::value;
    typedef boost::mpl::bool_< // CPU cache line is often 64
        (is_value_out && !is_value_empty) || (sizeof(Node) > 64)
    > sort_by_index;
    typedef boost::mpl::false_ sort_by_index_no;
    typedef boost::mpl::true_  sort_by_index_yes;

public:
    hash_strmap(HashFunc hashfn = HashFunc(), KeyEqual equalfn = KeyEqual())
      : hash(hashfn), equal(equalfn) {
        init();
    }
    explicit hash_strmap(size_t cap, HashFunc hashfn = HashFunc(), KeyEqual equalfn = KeyEqual())
      : hash(hashfn), equal(equalfn) {
        init();
        rehash(cap);
    }
    hash_strmap(const hash_strmap& y)
      : hash(y.hash), equal(y.equal) {
        pNodes = NULL;
        nNodes = y.nNodes - y.nDeleted;
        maxNodes = y.nNodes;
        maxload = y.maxload;
        nBucket = y.nBucket;
        nDeleted = 0;
        pHash = NULL;

        strpool = NULL;
        lenpool = y.lenpool;
        maxpool = y.lenpool;
        freepool = y.freepool;

        load_factor = y.load_factor;
        sort_flag = y.sort_flag;
        values = NULL;

        if (0 == nNodes) { // empty
            nBucket = 1;
            maxload = 0;
            bucket  = dummy_bucket<LinkTp>::b;
            if (intptr_t(y.pHash) == dont_cache_hash)
                pHash = (HashTp*)(dont_cache_hash);
            return; // not need to do deep copy
        }
        bucket = (LinkTp*)malloc(sizeof(LinkTp) * y.nBucket);
        if (NULL == bucket) {
            throw std::bad_alloc();
        }
        pNodes = (Node*)malloc(allnodes_size(nNodes));
        if (NULL == pNodes) {
            free(bucket);
            init(); // reset to safe state
            throw std::bad_alloc();
        }
        if (intptr_t(y.pHash) == dont_cache_hash) {
            pHash = (HashTp*)(dont_cache_hash);
        }
        else {
            pHash = (HashTp*)malloc(sizeof(HashTp) * nNodes);
            if (NULL == pHash) {
                free(pNodes);
                free(bucket);
                init(); // reset to safe state
                throw std::bad_alloc();
            }
        }
        strpool = (char*)malloc(y.lenpool);
        if (NULL == strpool) {
            if (intptr_t(pHash) != dont_cache_hash) free(pHash);
            free(pNodes);
            free(bucket);
            init(); // reset to safe state
            throw std::bad_alloc();
        }
        if (is_value_out &&	!is_value_empty) {
            values = (Value*)malloc(sizeof(Value) * nNodes);
            if (NULL == values) {
                if (intptr_t(pHash) != dont_cache_hash) free(pHash);
                free(strpool);
                free(pNodes);
                free(bucket);
                init(); // reset to safe state
                throw std::bad_alloc();
            }
        }
        if (0 == y.nDeleted) {
            if (intptr_t(pHash) != dont_cache_hash)
                memcpy(pHash, y.pHash, sizeof(HashTp) * nNodes);
            memcpy(strpool, y.strpool, sizeof(char) * lenpool);
            memcpy(bucket , y.bucket , sizeof(LinkTp) * nBucket);
            std::uninitialized_copy(y.pNodes, y.pNodes + nNodes, pNodes);
            pNodes[nNodes].offset = SAVE_OFFSET(lenpool);
            if (is_value_out &&	!is_value_empty)
                std::uninitialized_copy(y.values, y.values + nNodes, values);
            return;
        }
        size_t j = 0;
        // could compiler don't generate code the try ... catch if
        // it would not throw exceptions by static analysis?
        try {
            const Node* ypn = y.pNodes;
            size_t n = y.nNodes;
            size_t loffset = 0;
            for (size_t i = 0; i < n; ++i) {
                if (ypn[i].link != delmark) {
                    pNodes[j].offset = SAVE_OFFSET(loffset);
                    if (intptr_t(pHash) != dont_cache_hash)
                        pHash[j] = y.pHash[i];
                    new(&nth_value(j, ValuePlace()))Value(y.nth_value(i, ValuePlace()));
                    size_t beg2 = LOAD_OFFSET(ypn[i+0].offset);
                    size_t end2 = LOAD_OFFSET(ypn[i+1].offset);
                    size_t len2 = end2 - beg2;
                    memcpy(strpool + loffset, y.strpool + beg2, len2);
                    loffset += len2;
                    ++j;
                }
            }
            assert(j == nNodes);
            pNodes[j].offset = SAVE_OFFSET(loffset);
            pNodes[j].link = tail;
            lenpool = loffset;
            relink();
        }
        catch (...) {
            if (!boost::has_trivial_destructor<Value>::value) {
                while (j > 0) // roll back
                    nth_value(j-1, ValuePlace()).~Value(), --j;
            }
            if (is_value_out && !is_value_empty) {
                assert(NULL != values);
                free(values);
            }
            free(bucket);
            free(pNodes);
            free(strpool);
            init(); // reset to safe state
            throw;
        }
    }
    hash_strmap& operator=(const hash_strmap& y) {
        if (this != &y) {
        #if 1 // prefer performance, std::map is in this manner
        // this is exception-safe but lose old content of *this on exception
            this->destroy();
            new(this)hash_strmap(y);
        #else // exception-safe, but take more peak memory
            hash_strmap(y).swap(*this);
        #endif
        }
        return *this;
    }
    ~hash_strmap() { destroy(); }

    void swap(hash_strmap& y) {
        std::swap(pNodes  , y.pNodes);
        std::swap(nNodes  , y.nNodes);
        std::swap(maxNodes, y.maxNodes);
        std::swap(maxload , y.maxload);
        std::swap(nDeleted, y.nDeleted);
        std::swap(pHash   , y.pHash);

        std::swap(nBucket , y.nBucket);
        std::swap(bucket  , y.bucket);

        std::swap(strpool , y.strpool);
        std::swap(lenpool , y.lenpool);
        std::swap(maxpool , y.maxpool);
        std::swap(freepool, y.freepool);

        std::swap(load_factor, y.load_factor);
        std::swap(sort_flag, y.sort_flag);
        std::swap(hash     , y.hash);
        std::swap(equal    , y.equal);
    }

    void clear() {
        destroy();
        init();
    }

    void shrink_to_fit() {
        // before calling this function,
        //   the class-invariant(pHash/pLink relation) may not satisfied
        //   such as in this->erase_if..
        if (nDeleted)
            revoke_deleted_no_relink();
        if (maxpool != lenpool) {
            strpool = (char*)realloc(strpool, lenpool); // never fail
            maxpool = lenpool;
        }
        reserve_nodes(nNodes);
        rehash(size_t(nNodes / load_factor + 1));
    }

    void rehash(size_t newBucketSize) {
        newBucketSize = __hsm_stl_next_prime(newBucketSize);
        if (newBucketSize != nBucket) {
            // shrink or enlarge
            if (bucket != dummy_bucket<LinkTp>::b) {
                free(bucket);
            }
            bucket = (LinkTp*)malloc(sizeof(LinkTp) * newBucketSize);
            if (NULL == bucket) { // how to handle?
                assert(0); // immediately fail on debug mode
                clear();
                throw std::runtime_error("rehash failed, unrecoverable");
            //	throw std::runtime_error("rehash failed");
            }
            nBucket = newBucketSize;
            relink();
            using namespace std;
            maxload = LinkTp(min<double>(newBucketSize*load_factor, maxlink));
        }
    }
//	void resize(size_t n) { rehash(n); }

    // with rehash
    void reserve(size_t cap) {
        assert(cap >= nNodes);
        if (cap < nNodes)
            throw std::runtime_error(
        "hash_strmap::reserve(cap), cap is less than nNodes"
                );
        if (cap > maxlink)
            cap = maxlink;
        reserve_nodes(cap);
        rehash(size_t(cap / load_factor + 1));
    }

    // with rehash
    void reserve(size_t cap, size_t poolcap) {
        assert(cap >= nNodes);
        if (cap < nNodes)
            throw std::runtime_error(
        "hash_strmap::reserve(cap,poolcap), cap is less than nNodes"
            );
        if (poolcap < lenpool)
            throw std::runtime_error(
        "hash_strmap::reserve(cap,poolcap), poolcap is less than lenpool"
            );
// commented, because strpool realloc is cheap, Value realloc may expense,
// reserve more Value space may get benefit
//		if (poolcap < IF_SP_ALIGN(LOAD_OFFSET(1), 2) * cap)
//			throw std::logic_error(
//"hash_strmap::reserve(cap,poolcap), poolcap is less than cap expected"
//			);
        if (cap > maxlink)
            cap = maxlink;
        reserve_strpool(poolcap);
        reserve_nodes(cap);
        rehash(size_t(cap / load_factor + 1));
    }

    // just enlarge/shrink strpool
    void reserve_strpool(size_t poolcap) {
        assert(poolcap >= lenpool);
        if (poolcap < lenpool)
            throw std::runtime_error(
        "hash_strmap::reserve_strpool(poolcap), poolcap is less than lenpool"
            );
        char* ps = (char*)realloc(strpool, poolcap);
        if (NULL == ps)
            throw std::bad_alloc();
        strpool = ps;
        maxpool = poolcap;
    }

    // without rehash
    void reserve_nodes(size_t cap) {
        assert(cap >= nNodes);
        if (cap < nNodes)
            throw std::runtime_error(
        "hash_strmap::reserve_nodes(cap), cap is less than nNodes"
            );
        revoke_deleted();
        if (cap > maxlink)
            cap = maxlink;
        if (cap != (size_t)maxNodes) {
            if (intptr_t(pHash) != dont_cache_hash) {
                HashTp* ph = (HashTp*)realloc(pHash, sizeof(HashTp) * cap);
                if (NULL == ph)
                    throw std::bad_alloc();
                pHash = ph;
            }
            reserve_nodes_impl(cap, CopyStrategy(), ValuePlace());
            maxNodes = LinkTp(cap);
        }
    }

    void set_load_factor(double fact) {
        if (fact >= 0.999) {
            throw std::logic_error("load factor must <= 0.999");
        }
        load_factor = fact;
        using namespace std;
        maxload = dummy_bucket<LinkTp>::b == bucket ? 0
                : LinkTp(min<double>(nBucket * fact, maxlink));
    }
    void get_load_factor() const { return load_factor; }

    HashTp hash_value(size_t i) const {
        assert(i < nNodes);
        if (intptr_t(pHash) == dont_cache_hash)
            return hash(key(i));
        else
            return pHash[i];
    }

    bool is_hash_cached() const {
        return intptr_t(pHash) != dont_cache_hash;
    }

    void enable_hash_cache() {
        if (intptr_t(pHash) == dont_cache_hash) {
            if (0 == maxNodes) {
                pHash = NULL;
            }
            else {
                HashTp* ph = (HashTp*)malloc(sizeof(HashTp) * maxNodes);
                if (NULL == ph) {
                    throw std::bad_alloc();
                }
                size_t  n = nNodes;
                Node*  pn = pNodes;
                if (0 == nDeleted) {
                    for (size_t i = 0; i < n; ++i)
                        ph[i] = hash(key(i));
                }
                else {
                    for (size_t i = 0; i < n; ++i)
                        if (delmark != pn[i].link)
                            ph[i] = hash(key(i));
                }
                pHash = ph;
            }
        }
        assert(NULL != pHash || 0 == maxNodes);
    }

    void disable_hash_cache() {
        if (intptr_t(pHash) == dont_cache_hash)
            return;
        if (pHash)
            free(pHash);
        pHash = (HashTp*)(dont_cache_hash);
    }

    bool    empty() const { return nNodes== nDeleted; }
    size_t   size() const { return nNodes - nDeleted; }
    size_t  end_i() const { return nNodes; }
    size_t  beg_i() const {
    //	return next_i(intptr_t(-1)); assertion will fail in next_i
    //	because size_t is unsigned, so write the code below
    //	this code produce same result as next_i(intptr_t(-1))
       	const Node* pn = pNodes;
       	size_t i = 0;
       	while (delmark == pn[i].link)
            ++i;
        return i;
    }
    size_t delcnt() const { return nDeleted; }

    std::pair<size_t, bool> insert_i(const std::pair<fstring, Value>& kv) {
        return insert_i(kv.first, kv.second);
    }

    Value& operator[](const fstring key) {
        std::pair<size_t, bool> ib = insert_i(key, Value());
        return nth_value(ib.first, ValuePlace());
    }
    const Value& operator[](const fstring key) const {
        size_t i = find_i(key);
        if (i == nNodes) {
            throw std::logic_error(
                "hash_strmap::operator[] const: key=\"" +
                std::string(key.p, key.n) + "\" doesn't exist");
        }
        return nth_value(i, ValuePlace());
    }
    const Value& val(size_t idx) const {
        HSM_SANITY(nNodes >= 1);
        assert(idx < nNodes);
        return nth_value(idx, ValuePlace());
    }
    Value& val(size_t idx) {
        HSM_SANITY(nNodes >= 1);
        assert(idx < nNodes);
        return nth_value(idx, ValuePlace());
    }

          iterator  begin()       { return       iterator(this, beg_i()); }
    const_iterator  begin() const { return const_iterator(this, beg_i()); }
    const_iterator cbegin() const { return const_iterator(this, beg_i()); }

          iterator  end()       { return       iterator(this, nNodes); }
    const_iterator  end() const { return const_iterator(this, nNodes); }
    const_iterator cend() const { return const_iterator(this, nNodes); }

          reverse_iterator  rbegin()       { return       reverse_iterator(end()); }
    const_reverse_iterator  rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }

          reverse_iterator  rend()       { return       reverse_iterator(begin()); }
    const_reverse_iterator  rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    std::pair<iterator, bool>
    insert(const std::pair<const fstring, Value>& kv) {
        std::pair<size_t, bool> ib = insert_i(kv);
        return std::pair<iterator, bool>(iterator(this, ib.first), ib.second);
    }
          iterator find(fstring key)       { return       iterator(this, find_i(key)); }
    const_iterator find(fstring key) const { return const_iterator(this, find_i(key)); }

    // return erased element count
    size_t erase(const fstring key) {
        assert(key.n >= 0);
        size_t h = hash(key);
        size_t i = h % nBucket;
        for (LinkTp p = bucket[i]; tail != p; p = pNodes[p].link) {
            HSM_SANITY(p < nNodes);
            const Node* y = &pNodes[p];
            HSM_SANITY(y[0].offset < y[1].offset);
            HSM_SANITY(y[1].offset <= SAVE_OFFSET(maxpool));
        // doesn't need to compare cached hash value, it almost always true
            size_t ybeg = LOAD_OFFSET(y[0].offset);
            size_t yend = LOAD_OFFSET(y[1].offset);
            size_t ylen = yend - ybeg - extralen(yend);
            if (equal(key, fstring(strpool + ybeg, ylen))) {
                erase_i_impl(p, h);
                return 1;
            }
        }
        return 0;
    }

    void erase(iterator iter) {
        assert(iter.get_owner() == this);
        assert(iter.get_index() < nNodes);
        erase_i(iter.get_index());
    }
private:
    template<class Predictor>
    size_t erase_if_kv_impl(Predictor pred) {
        Value* pv = values;
        HashTp*ph = pHash;
        Node* pn = pNodes;
        char* ps = strpool;
        size_t n = nNodes;
        size_t i = 0;
        size_t loffset;
        if (0 == n)
            return 0;
        assert(intptr_t(ph) == dont_cache_hash || NULL != ph);
        if (0 == nDeleted) {
            for (; i < n; ++i) {
                HSM_SANITY(pn[i+0].offset < pn[i+1].offset);
                HSM_SANITY(pn[i+1].offset <= SAVE_OFFSET(maxpool));
                const size_t mybeg = LOAD_OFFSET(pn[i+0].offset);
                const size_t myend = LOAD_OFFSET(pn[i+1].offset);
                const size_t mylen = myend - mybeg;
                const size_t extra = extralen(myend);
                const fstring mykey(ps + mybeg, mylen-extra);
                Value& myval = nth_value(pn, pv, i, ValuePlace()); // non-const
                if (pred(mykey, myval))
                    goto DoErase1;
            }
            return 0;
        DoErase1:
            nth_value(pn, pv, i, ValuePlace()).~Value();
            loffset = LOAD_OFFSET(pn[i].offset); // begin of first deleted
            for (size_t j = i+1; j < n; ++j) {
                HSM_SANITY(pn[j+0].offset < pn[j+1].offset);
                HSM_SANITY(pn[j+1].offset <= SAVE_OFFSET(maxpool));
                const size_t mybeg = LOAD_OFFSET(pn[j+0].offset);
                const size_t myend = LOAD_OFFSET(pn[j+1].offset);
                const size_t mylen = myend - mybeg;
                const size_t extra = extralen(myend);
                const fstring mykey(ps + mybeg, mylen-extra);
                Value& myval = nth_value(pn, pv, j, ValuePlace()); // non-const
                if (pred(mykey, myval))
                    nth_value(pn, pv, j, ValuePlace()).~Value(); // destruct
                else {
                    pn[i].offset = SAVE_OFFSET(loffset);
                    move_value(pn+i, pn+j, pv+i, pv+j, ValuePlace());
                    memcpy(ps + loffset, ps + mybeg, mylen);
                    if (intptr_t(ph) != dont_cache_hash)
                        ph[i] = ph[j];
                    loffset += mylen;
                    ++i;
                }
            }
        }
        else { // 0 != nDeleted
            for (; i < n; ++i) {
                HSM_SANITY(pn[i+0].offset < pn[i+1].offset);
                HSM_SANITY(pn[i+1].offset <= SAVE_OFFSET(maxpool));
                const size_t mybeg = LOAD_OFFSET(pn[i+0].offset);
                const size_t myend = LOAD_OFFSET(pn[i+1].offset);
                const size_t mylen = myend - mybeg;
                const size_t extra = extralen(myend);
                const fstring mykey(ps + mybeg, mylen-extra);
                Value& myval = nth_value(pn, pv, i, ValuePlace()); // non-const
                if (delmark == pn[i].link || pred(mykey, myval))
                    goto DoErase2;
            }
            return 0;
        DoErase2:
            nth_value(pn, pv, i, ValuePlace()).~Value();
            loffset = LOAD_OFFSET(pn[i].offset); // begin of first deleted
            for (size_t j = i+1; j < n; ++j) {
                HSM_SANITY(pn[j+0].offset < pn[j+1].offset);
                HSM_SANITY(pn[j+1].offset <= SAVE_OFFSET(maxpool));
                const size_t mybeg = LOAD_OFFSET(pn[j+0].offset);
                const size_t myend = LOAD_OFFSET(pn[j+1].offset);
                const size_t mylen = myend - mybeg;
                const size_t extra = extralen(myend);
                const fstring mykey(ps + mybeg, mylen-extra);
                Value& myval = nth_value(pn, pv, j, ValuePlace()); // non-const
                if (delmark == pn[j].link)
                    ; // do nothing
                else if (pred(mykey, myval))
                    nth_value(pn, pv, j, ValuePlace()).~Value(); // destruct
                else {
                    pn[i].offset = SAVE_OFFSET(loffset);
                    move_value(pn+i, pn+j, pv+i, pv+j, ValuePlace());
                    memcpy(ps + loffset, ps + mybeg, mylen);
                    if (intptr_t(ph) != dont_cache_hash)
                        ph[i] = ph[j];
                    loffset += mylen;
                    ++i;
                }
            }
        }
        size_t nDeleted0 = nDeleted;
        lenpool = loffset;
        nNodes = LinkTp(i);
        pn[i].offset = SAVE_OFFSET(loffset);
        pn[i].link = tail; // guard for next_i
        nDeleted = 0;
        freepool = 0;
        return n - nDeleted0 - i;
    }
public:
    template<class Predictor>
    size_t erase_if_kv(Predictor pred) {
        size_t nErased = erase_if_kv_impl(pred);
        if (nNodes * 3/2 <= maxNodes)
            shrink_to_fit();
        else
            relink();
        return nErased;
    }
    template<class Predictor>
    size_t shrink_after_erase_if_kv(Predictor pred) {
        size_t nErased = erase_if_kv_impl(pred);
        shrink_to_fit();
        return nErased;
    }
    template<class Predictor>
    size_t no_shrink_after_erase_if_kv(Predictor pred) {
        size_t nErased = erase_if_kv_impl(pred);
        relink();
        return nErased;
    }
private:
    template<class Predictor>
    struct PPairToKV {
        bool operator()(const fstring& k, Value& v) {
            value_type kv(k, v);
            return pred(kv);
        }
        PPairToKV(Predictor pred_) : pred(pred_) {}
        Predictor pred;
    };
public:
    template<class Predictor>
    size_t erase_if(Predictor pred) {
        return erase_if_kv(PPairToKV<Predictor>(pred));
    }
    template<class Predictor>
    size_t shrink_after_erase_if(Predictor pred) {
        return shrink_after_erase_if_kv(PPairToKV<Predictor>(pred));
    }
    template<class Predictor>
    size_t no_shrink_after_erase_if(Predictor pred) {
        return no_shrink_after_erase_if_kv(PPairToKV<Predictor>(pred));
    }
private:
    void revoke_deleted_no_relink() throw() {
        assert(nDeleted > 0);
        ptrdiff_t n = nNodes;
        char* ps = strpool;
        Node* pn = pNodes;
        Value* pv = values;
        HashTp*ph = pHash;
        ptrdiff_t idx1 = 0;
        for (; idx1 < n; ++idx1) {
            if (pn[idx1].link == delmark)
                break; // first deleted
        }
        size_t loffset = LOAD_OFFSET(pn[idx1].offset); // begin of first deleted
        for (ptrdiff_t idx2 = idx1 + 1; idx2 < n; ++idx2) {
            if (pn[idx2].link != delmark) { // is not deleted
                // string copy should be as fast as possible
            #if defined(SP_ALIGN) && SP_ALIGN != 0 && defined(HSM_ENABLE_MY_memcpy)
                size_t beg2 = pn[idx2+0].offset;
                size_t end2 = pn[idx2+1].offset;
                size_t len2 = end2 - beg2; // count by align_type
                HSM_SANITY(len2 > 0);
                align_type* dst = (align_type*)(ps + loffset);
                align_type* src = (align_type*)(ps + LOAD_OFFSET(beg2));
                while (len2--) *dst++ = *src++;
                len2 = LOAD_OFFSET(end2 - beg2);
            #else
                size_t beg2 = LOAD_OFFSET(pn[idx2+0].offset);
                size_t end2 = LOAD_OFFSET(pn[idx2+1].offset);
                size_t len2 = end2 - beg2;
                memcpy(ps + loffset, ps + beg2, len2);
            #endif
                pn[idx1].offset = SAVE_OFFSET(loffset);
                move_value(pn+idx1, pn+idx2, pv+idx1, pv+idx2, ValuePlace());
                if (intptr_t(ph) != dont_cache_hash)
                    ph[idx1] = ph[idx2];
               	loffset += len2;
                ++idx1;
            }
        }
        assert(loffset + freepool == lenpool);
        assert(nNodes  - nDeleted == idx1);
        lenpool = loffset;
        pn[idx1].offset = SAVE_OFFSET(loffset);
        pn[idx1].link = tail; // guard for next_i

        nNodes -= nDeleted;
        nDeleted = 0;
        freepool = 0;
    }

public:
    void revoke_deleted() throw() {
        if (nDeleted) {
            revoke_deleted_no_relink();
            relink();
        }
    }

    size_t next_i(size_t idx) const {
        assert(idx < nNodes);
        Node* pn = pNodes;
    #if 0
        // don't use pn[nNodes].link guard
        size_t n = nNodes;
        do ++idx;
        while (idx < n && pn[idx].link == delmark);
    #else
        // pn[nNodes].link is the guard
        do ++idx;
        while (pn[idx].link == delmark);
    #endif
        assert(idx <= nNodes);
        return idx;
    }

    /**
    A valid reverse iteration by prev_i should be:
    <code>
    for (size_t i = fm.end_i(), re = fm.beg_i(); i != re;) {
        i = fm.prev_i(i);
        cout << fm.key(i) << ',' << fm.val(i) << endl;
    }
    // or by reverse_iterator:
    // reverse_iterator need 2 prev_i call per iteration
    for (auto i = fm.rbegin(); i != fm.rend(); ++i) {
        // using i
    }
    </code>
    */
    size_t prev_i(size_t idx) const {
        assert(idx >  0); // not the begin
        assert(idx <= nNodes);
        Node* pn = pNodes;
    #if defined(_DEBUG) || !defined(NDEBUG)
        size_t n = nNodes;
        do --idx; // no guard for prev_i, if idx rolled from 0, the loop ended
        while (idx < n && pn[idx].link == delmark);

        // if these 2 assert fail,
        //    it should caused by iterate over begin when pNodes[0] is deleted
        assert(idx < nNodes);
        assert(pn[idx].link != delmark);
    #else
        // no extra guard is need for prev_i
        // if pn[0] was deleted, a valid caller should not make idx less than 0
        // because valid caller will not past beg_i() by calling prev_i
        do --idx;
        while (pn[idx].link == delmark);
    #endif
        return idx;
    }

    std::pair<size_t, bool> insert_i(const fstring key, const Value& val) {
        assert(key.n >= 0);
        size_t n = nNodes; // compiler is easier to track local var
        size_t h = hash(key);
        size_t i = h % nBucket;
        for (LinkTp p = bucket[i]; tail != p; p = pNodes[p].link) {
            HSM_SANITY(p < n);
            const Node* y = &pNodes[p];
            HSM_SANITY(y[0].offset < y[1].offset);
            HSM_SANITY(y[1].offset <= SAVE_OFFSET(maxpool));
        // doesn't need to compare cached hash value, it almost always true
            size_t ybeg = LOAD_OFFSET(y[0].offset);
            size_t yend = LOAD_OFFSET(y[1].offset);
            size_t ylen = yend - ybeg - extralen(yend);
           	if (equal(key, fstring(strpool + ybeg, ylen)))
                return std::make_pair(p, false);
        }
        if (HSM_unlikely(n >= maxload)) {
            rehash(nBucket * 2);
            i = h % nBucket;
        }
        HSM_SANITY(n <= maxNodes);
        if (HSM_unlikely(n == maxNodes)) {
            if (HSM_unlikely(maxlink == n)) {
                throw std::runtime_error("nNodes reaches maxlink");
            }
            reserve_nodes(0 == n ? 1 : 2*n);
        }
        size_t real_len = IF_SP_ALIGN(fstring::align_to, size_t)(key.n+1);
        if (HSM_unlikely(lenpool + real_len > maxpool)) {
            using namespace std; // for std::max
            if (lenpool + real_len > maxoffset) {
                throw std::runtime_error("strpool is exceeding maxoffset");
            }
            if (freepool >= max(maxpool/2, real_len))
                revoke_deleted();
            else { // if lenpool is random, expansion scale is about 1.6
                size_t newmax = min(__hsm_align_pow2((lenpool + real_len)*5/4), maxoffset);
                char*  newpch = (char*)realloc(strpool, newmax);
                if (NULL == newpch) {
                    if (freepool >= real_len)
                        revoke_deleted();
                    else
                        throw std::bad_alloc();
                } else {
                    maxpool = newmax;
                    strpool = newpch;
                }
            }
            n = nNodes; // revoke_deleted may update nNodes
        }
#if defined(SP_ALIGN) && SP_ALIGN != 0
        ((align_type*)(strpool + lenpool + real_len))[-1] = 0; // pad 0
        strpool[lenpool + real_len - 1] = (char)(real_len - key.n - 1);
#else
        strpool[lenpool + key.n] = '\0'; // the key should be used as c_str
#endif
        memcpy(strpool + lenpool, key.p, key.n);
        lenpool += real_len;
        new(&nth_value(n, ValuePlace()))Value(val);
        pNodes[n].link = bucket[i]; // newer at head
        pNodes[n+1].offset = SAVE_OFFSET(lenpool);
        pNodes[n+1].link = tail; // guard for next_i
        bucket[i] = LinkTp(n); // new head of i'th bucket
        if (intptr_t(pHash) != dont_cache_hash) {
            assert(NULL != pHash);
            pHash[n] = h;
        }
        sort_flag = en_unsorted;
        return std::make_pair(nNodes++, true);
    }

    size_t find_i(const fstring key) const {
        assert(key.n >= 0);
        size_t n = nNodes; // compiler is easier to track local var
        size_t h = hash(key);
        size_t i = h % nBucket;
        for (LinkTp p = bucket[i]; tail != p; p = pNodes[p].link) {
            HSM_SANITY(p < n);
            const Node* y = &pNodes[p];
            HSM_SANITY(y[0].offset < y[1].offset);
            HSM_SANITY(y[1].offset <= SAVE_OFFSET(maxpool));
        // doesn't need to compare cached hash value, it almost always true
            size_t ybeg = LOAD_OFFSET(y[0].offset);
            size_t yend = LOAD_OFFSET(y[1].offset);
            size_t ylen = yend - ybeg - extralen(yend);
           	if (equal(key, fstring(strpool + ybeg, ylen)))
                return p;
        }
        return n; // not found
    }

    size_t count(const fstring key) const {
        assert(key.n >= 0);
        return find_i(key) == nNodes ? 0 : 1;
    }
    // return value is same with count(key), but type is different
    // this function is not stl standard, but more meaningful for app
    bool exists(const fstring key) const {
        assert(key.n >= 0);
        return find_i(key) != nNodes;
    }
private:
    void erase_i_impl(const size_t idx, const size_t h) {
        size_t const i = h % nBucket;
        HSM_SANITY(tail != bucket[i]);
        LinkTp* p = &bucket[i];
        while (idx != *p) {
            p = &pNodes[*p].link;
            HSM_SANITY(*p < nNodes);
        }
        *p = pNodes[idx].link; // delete the node from link list
        pNodes[idx].link = delmark; // set deletion mark
        nth_value(idx, ValuePlace()).~Value(); // destroy
        size_t mybeg = LOAD_OFFSET(pNodes[idx+0].offset);
        size_t myend = LOAD_OFFSET(pNodes[idx+1].offset);
        freepool += myend - mybeg;
        ++nDeleted;
        if (nDeleted >= nNodes/2)
            revoke_deleted();

    //	relaxed: allow for key sorted and POD value sorted
        if (!boost::has_trivial_destructor<Value>::value && en_sort_by_val == sort_flag)
            sort_flag = en_unsorted;
    }
public:
    void erase_i(const size_t idx) {
        HSM_SANITY(nNodes >= 1);
        assert(idx < nNodes);
        assert(pNodes[idx].link != delmark); // must has not deleted
        assert(intptr_t(pHash) == dont_cache_hash || NULL != pHash);
        size_t h = intptr_t(pHash) == dont_cache_hash ? hash(key(idx)) : pHash[idx];
        erase_i_impl(idx, h);
    }

    bool is_deleted(size_t idx) const {
        assert(nNodes >= 1);
        assert(idx < nNodes);
        return pNodes[idx].link == delmark;
    }

    const char* key_c_str(size_t idx) const {
        HSM_SANITY(nNodes >= 1);
        assert(idx < nNodes);
        size_t mybeg = LOAD_OFFSET(pNodes[idx+0].offset);
        HSM_SANITY(mybeg < lenpool);
        return strpool + mybeg;
    }
    size_t key_len(size_t idx) const {
        HSM_SANITY(nNodes >= 1);
        assert(idx < nNodes);
        size_t mybeg = LOAD_OFFSET(pNodes[idx+0].offset);
        size_t myend = LOAD_OFFSET(pNodes[idx+1].offset);
        size_t extra = extralen(myend);
        size_t mylen = myend - mybeg - extra;
        HSM_SANITY(mybeg < myend);
        HSM_SANITY(myend <= lenpool);
        return mylen;
    }
    fstring key(size_t idx) const {
        HSM_SANITY(nNodes >= 1);
        assert(idx < nNodes);
        size_t mybeg = LOAD_OFFSET(pNodes[idx+0].offset);
        size_t myend = LOAD_OFFSET(pNodes[idx+1].offset);
        size_t extra = extralen(myend);
        size_t mylen = myend - mybeg - extra;
        HSM_SANITY(mybeg < myend);
        HSM_SANITY(myend <= lenpool);
        return fstring(strpool + mybeg, mylen);
    }

private:
    template<class ValueRef, class Func>
    void for_each_impl(Func f) const {
        Node* pn = pNodes;
        char* ps = strpool;
        if (nDeleted) {
            for (size_t i = 0, n = nNodes; i < n; ++i) {
                // need check deletion mark
                if (pn[i].link != delmark) {
                    const size_t mybeg = LOAD_OFFSET(pn[i+0].offset);
                    const size_t myend = LOAD_OFFSET(pn[i+1].offset);
                    const size_t extra = extralen(ps, myend);
                    const size_t mylen = myend - mybeg - extra;
                    HSM_SANITY(mybeg < myend);
                    HSM_SANITY(myend <= lenpool);
                    const fstring key(ps + mybeg, mylen);
                    ValueRef val = nth_value(i, ValuePlace());
                    f(key, val);
                }
            }
        }
       	else { // need not to check deletion mark
            for (size_t i = 0, n = nNodes; i < n; ++i) {
                const size_t mybeg = LOAD_OFFSET(pn[i+0].offset);
                const size_t myend = LOAD_OFFSET(pn[i+1].offset);
                const size_t extra = extralen(ps, myend);
                const size_t mylen = myend - mybeg - extra;
                HSM_SANITY(mybeg < myend);
                HSM_SANITY(myend <= lenpool);
                const fstring key(ps + mybeg, mylen);
                ValueRef val = nth_value(i, ValuePlace());
                f(key, val);
            }
        }
    }
public:
    template<class Func>
    void for_each(Func f) const {
        for_each_impl<const Value&, Func>(f);
    }
    template<class Func>
    void for_each(Func f) {
        for_each_impl<Value&, Func>(f);
    }

// Sorting and Binary search support:
private:
    struct KeyIndex {
        // use this struct instead of just a 'int' index to pNodes could
        // saved 1 indirect memory access to pNodes[*], friendly to CPU cache
        LinkTp offset;
        LinkTp length;
        LinkTp idx; // index to pNodes
    };
    template<class KeyCompare>
    struct SortKeyCompare : KeyCompare {
        const char* ps;
        bool operator()(const KeyIndex& x, const KeyIndex& y) const {
            fstring sx(ps + LOAD_OFFSET(x.offset), x.length);
            fstring sy(ps + LOAD_OFFSET(y.offset), y.length);
            return KeyCompare::operator()(sx, sy);
        }
        bool operator()(const Node& x, const Node& y) const {
            fstring sx(ps + LOAD_OFFSET(x.offset), x.link); // link is length
            fstring sy(ps + LOAD_OFFSET(y.offset), y.link);
            return KeyCompare::operator()(sx, sy);
        }
        SortKeyCompare(KeyCompare cmp, const char* ps) : KeyCompare(cmp), ps(ps) {}
    };

    void save_strlen_to_link() {
        Node* pn = pNodes;
        IF_SP_ALIGN(char* ps = strpool,;);
        for (size_t i = 0, n = nNodes; i < n; ++i) {
            const size_t mybeg = LOAD_OFFSET(pn[i+0].offset);
            const size_t myend = LOAD_OFFSET(pn[i+1].offset);
            const size_t extra = extralen(ps, myend);
            const size_t mylen = myend - mybeg - extra;
            IF_SP_ALIGN(HSM_SANITY(extra <= SP_ALIGN),;);
            HSM_SANITY(mybeg <  myend);
            HSM_SANITY(myend <= lenpool);
            pn[i].link = LinkTp(mylen); // 'link' save the strlen
        }
    }

    KeyIndex* buildindex() {
        KeyIndex* index = (KeyIndex*)malloc(sizeof(KeyIndex) * nNodes);
        if (NULL == index) {
            throw std::bad_alloc();
        }
        Node* pn = pNodes;
        IF_SP_ALIGN(char* ps = strpool,;);
        for (size_t i = 0, n = nNodes; i < n; ++i) {
            const size_t mybeg = LOAD_OFFSET(pn[i+0].offset);
            const size_t myend = LOAD_OFFSET(pn[i+1].offset);
            const size_t extra = extralen(ps, myend);
            const size_t mylen = myend - mybeg - extra;
            IF_SP_ALIGN(HSM_SANITY(extra <= SP_ALIGN),;);
            HSM_SANITY(mybeg <  myend);
            HSM_SANITY(myend <= lenpool);
            index[i].offset = SAVE_OFFSET(mybeg);
            index[i].length = pn[i].link = LinkTp(mylen);
            index[i].idx = LinkTp(i);
        }
        return index;
    }

    LinkTp* buildindex_by_int() {
        LinkTp* p = (LinkTp*)malloc(sizeof(LinkTp) * nNodes);
        if (NULL == p) {
            throw std::bad_alloc();
        }
        for (size_t i = 0, n = nNodes; i < n; ++i)
            p[i] = i;
        return p;
    }

    void rearrange_nodes_by_int(LinkTp* index) {
        Node* pn = pNodes;
        Value* pv = values;
        HashTp*ph = pHash;
        for (size_t i = 0, n = nNodes; i < n; ++i) { // Rearrange
            if (index[i] != i) {
                LinkTp curr, next = index[i];
                LinkTp tmpoffset = pn[next].offset;
                LinkTp tmplink   = pn[next].link; // link saved strlen
                HashTp tmphash   = intptr_t(ph) == dont_cache_hash ? 0 : ph[next];
                char   tmpvalue[sizeof(Value)];
                move_cons(tmpvalue, &nth_value(next, ValuePlace()), CopyStrategy());
                do {
                    assert(next < n);
                    curr = next;
                    next = index[next];
                    pn[curr].offset = pn[next].offset;
                    pn[curr].link   = pn[next].link;
                    move_value(pn+curr, pn+next, pv+curr, pv+next, ValuePlace());
                    index[curr] = curr;
                    if (intptr_t(ph) != dont_cache_hash)
                        ph[curr] = ph[next];
                } while (next != i);
                move_cons(&nth_value(i, ValuePlace()), (Value*)tmpvalue, CopyStrategy());
                pn[i].offset = tmpoffset;
                pn[i].link   = tmplink;
                if (intptr_t(ph) != dont_cache_hash)
                    ph[i] = tmphash;
                index[i] = LinkTp(i);
            }
        }
        free(index);
    }

    void rearrange_nodes(KeyIndex* index) {
        // copy idx to LinkTp array, small array has benefit of CPU cache
        // in rearrange_nodes_by_int
        LinkTp* pi = (LinkTp*)index;
        for (size_t i = 0, n = nNodes; i < n; ++i)
            pi[i] = index[i].idx;
        rearrange_nodes_by_int(pi); // pi == index will be free'ed
    }

    void rearrange_strpool() {
        char* s2 = (char*)malloc(lenpool);
        if (NULL == s2) {
            throw std::bad_alloc();
        }
        size_t loffset = 0;
        char* ps = strpool;
        Node* pn = pNodes;
        for (size_t i = 0, n = nNodes; i < n; ++i) {
            size_t len = pn[i].link; // Node::link saved length
            len = IF_SP_ALIGN(fstring::align_to, size_t)(len + 1);
            // could be optimized as aligned copy?
            memcpy(s2 + loffset, ps + LOAD_OFFSET(pn[i].offset), len);
            pn[i].offset = SAVE_OFFSET(loffset);
            loffset += len;
        }
        assert(loffset == lenpool);
        free(strpool);
        strpool = s2;
        maxpool = lenpool;
    }

    template<class KeyCompare>
    void sort_by_key_impl(const KeyCompare& comp, sort_by_index_yes) {
        KeyIndex* index = buildindex();
        std::sort(index, index + nNodes, SortKeyCompare<KeyCompare>(comp, strpool));
        rearrange_nodes(index);
    }
    template<class KeyCompare>
    void sort_by_key_impl(const KeyCompare& comp, sort_by_index_no) {
        save_strlen_to_link();
        std::sort(pNodes, pNodes + nNodes, SortKeyCompare<KeyCompare>(comp, strpool));
    }

public:
    template<class KeyCompare>
    void sort(const KeyCompare& comp) {
        if (nDeleted)
            revoke_deleted_no_relink();
        if (nNodes >= 2) {
            sort_by_key_impl(comp, sort_by_index());
            rearrange_strpool(); // must after rearrange_nodes
            relink_fill();
        }
        sort_flag = en_sort_by_key;
    }
    void sort() { sort(fstring::IF_SP_ALIGN(less_align, less_unalign)()); }

    size_t lower_bound(const fstring key) const {
        return lower_bound(key, fstring::IF_SP_ALIGN(Less, less_unalign)());
    }
    size_t upper_bound(const fstring key) const {
        return upper_bound(key, fstring::IF_SP_ALIGN(Less, less_unalign)());
    }

    template<class KeyCompare>
    size_t lower_bound(const fstring key, const KeyCompare& comp) const {
        assert(en_sort_by_key == sort_flag);
        const Node* pn = &pNodes[0];
        const char* ps = strpool;
        size_t lo = 0, hi = nNodes;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2; // avoid overflow
            size_t beg = LOAD_OFFSET(pn[mid+0].offset);
            size_t End = LOAD_OFFSET(pn[mid+1].offset);
            size_t len = End - beg - extralen(ps, End);
            const fstring smid(ps + beg, len);
            if (comp(smid, key))
                lo = mid + 1;
            else
                hi = mid;
        }
        return lo;
    }

    template<class KeyCompare>
    size_t upper_bound(const fstring key, const KeyCompare& comp) const {
        assert(en_sort_by_key == sort_flag);
        const Node* pn = &pNodes[0];
        const char* ps = strpool;
        size_t lo = 0, hi = nNodes;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2; // avoid overflow
            size_t beg = LOAD_OFFSET(pn[mid+0].offset);
            size_t End = LOAD_OFFSET(pn[mid+1].offset);
            size_t len = End - beg - extralen(ps, End);
            const fstring smid(ps + beg, len);
            if (!comp(key, smid)) // smid <= key
                lo = mid + 1;
            else
                hi = mid;
        }
        return lo;
    }

    /// can be used to find common prefix ranges
    /// param: comp3 3-way compare, like strcmp
    template<class KeyPrefixCompare3>
    std::pair<size_t, size_t>
    equal_range3(const fstring key, const KeyPrefixCompare3& comp3) const {
        assert(en_sort_by_key == sort_flag);
        const Node* pn = &pNodes[0];
        const char* ps = strpool;
        size_t lo = 0, hi = nNodes;
        size_t mid0 = hi;
        while (lo < hi) {
            mid0 = lo + (hi - lo) / 2; // avoid overflow
            size_t beg = LOAD_OFFSET(pn[mid0+0].offset);
            size_t End = LOAD_OFFSET(pn[mid0+1].offset);
            size_t len = End - beg - extralen(ps, End);
            const fstring smid0(ps + beg, len);
            int ret = comp3(smid0, key);
            if (ret < 0)
                lo = mid0 + 1;
            else if (ret > 0)
                hi = mid0;
            else
                break;
        }
        size_t loH = mid0;
        while (lo < loH) {
            size_t mid = lo + (loH - lo) / 2; // avoid overflow
            size_t beg = LOAD_OFFSET(pn[mid+0].offset);
            size_t End = LOAD_OFFSET(pn[mid+1].offset);
            size_t len = End - beg - extralen(ps, End);
            const fstring smid(ps + beg, len);
            int ret = comp3(smid, key);
            if (ret < 0) // lower_bound
                lo = mid + 1;
            else
                loH = mid;
        }
        size_t hiL = mid0;
        while (hiL < hi) {
            size_t mid = hiL + (hi - hiL) / 2; // avoid overflow
            size_t beg = LOAD_OFFSET(pn[mid+0].offset);
            size_t End = LOAD_OFFSET(pn[mid+1].offset);
            size_t len = End - beg - extralen(ps, End);
            const fstring smid(ps + beg, len);
            int ret = comp3(smid, key);
            if (ret <= 0) // upper_bound
                hiL = mid + 1;
            else
                hi = mid;
        }
        return std::pair<size_t, size_t>(lo, hi);
    }
    std::pair<size_t, size_t>
    equal_range3(const char* prefix, size_t prefixlen) const {
        return equal_range3(fstring(prefix, prefixlen), fstring::prefix_compare3(prefixlen));
    }

// sort and binary search by value...
private:
    template<class CompareValue>
    struct SortCompareValueInline : CompareValue {
        bool operator()(const Node& x, const Node& y) const {
            return CompareValue::operator()(x.value, y.value);
        }
        SortCompareValueInline(CompareValue cmp) : CompareValue(cmp) {}
    };
    template<class CompareValue>
    struct SortCompareValueInlineByIndex : CompareValue {
        const Node* pn;
        bool operator()(const size_t x, const size_t y) const {
            return CompareValue::operator()(pn[x].value, pn[y].value);
        }
        SortCompareValueInlineByIndex(CompareValue cmp, const Node* pn) : CompareValue(cmp), pn(pn) {}
    };
    template<class CompareValue>
    struct SortCompareValueOut : CompareValue {
        const Value* pv;
        bool operator()(const size_t x, const size_t y) const {
            return CompareValue::operator()(pv[x], pv[y]);
        }
        SortCompareValueOut(CompareValue cmp, const Value* pv) : CompareValue(cmp), pv(pv) {}
    };

    template<class ValueCompare>
    void sort_by_value_impl(ValueCompare comp, ValueInline, sort_by_index_yes) {
        LinkTp* index = NULL;
        try { index = buildindex_by_int(); } catch (const std::bad_alloc&) {}
        if (index) {
            std::sort(index, index + nNodes, SortCompareValueInlineByIndex<ValueCompare>(comp, pNodes));
            rearrange_nodes_by_int(index);
            rearrange_strpool(); // must after rearrange_nodes
            relink();
        }
        else // alloc fail, fall back to in-place sort
            sort_by_value_impl(comp, ValueInline(), sort_by_index_no());
    }
    template<class ValueCompare>
    void sort_by_value_impl(ValueCompare comp, ValueInline, sort_by_index_no) {
        std::sort(pNodes, pNodes + nNodes, SortCompareValueInline<ValueCompare>(comp));
        rearrange_strpool(); // must after rearrange_nodes
        relink_fill();
    }
    template<class ValueCompare>
    void sort_by_value_impl(ValueCompare comp, ValueOut, sort_by_index_yes) {
        LinkTp* index = buildindex_by_int();
        Value* pv = values;
        std::sort(index, index + nNodes, SortCompareValueOut<ValueCompare>(comp, pv));
        rearrange_nodes_by_int(index);
        rearrange_strpool(); // must after rearrange_nodes
        relink();
    }

    template<class KeyOfValue, class Compare>
    struct FindCompareValueInline : Compare {
        bool operator()(const KeyOfValue& x, const Node& y) const {
            return Compare::operator()(x, y.value);
        }
        bool operator()(const Node& x, const KeyOfValue& y) const {
            return Compare::operator()(x.value, y);
        }
        bool operator()(const Node& x, const Node& y) const {
            // in debug mode, vc will check order of sequence when binary search,
            // so this function is required for this purpose
            return Compare::operator()(x.value, y.value);
        }
        FindCompareValueInline(Compare comp) : Compare(comp) {}
    };

    template<class KeyOfValue, class Compare>
    size_t lower_bound_by_value_impl(KeyOfValue kov, Compare comp, ValueOut) const {
        const Value* pv = values;
        return std::lower_bound(pv, pv + nNodes, kov, comp) - pv;
    }
    template<class KeyOfValue, class Compare>
    size_t upper_bound_by_value_impl(KeyOfValue kov, Compare comp, ValueOut) const {
        const Value* pv = values;
        return std::upper_bound(pv, pv + nNodes, kov, comp) - pv;
    }
    template<class KeyOfValue, class Compare>
    size_t lower_bound_by_value_impl(KeyOfValue kov, Compare comp, ValueInline) const {
        const Node* pn = pNodes;
        return std::lower_bound(pn, pn + nNodes, kov, FindCompareValueInline<KeyOfValue, Compare>(comp)) - pn;
    }
    template<class KeyOfValue, class Compare>
    size_t upper_bound_by_value_impl(KeyOfValue kov, Compare comp, ValueInline) const {
        const Node* pn = pNodes;
        return std::upper_bound(pn, pn + nNodes, kov, FindCompareValueInline<KeyOfValue, Compare>(comp)) - pn;
    }

    template<class KeyOfValue, class Compare>
    std::pair<size_t, size_t>
    equal_range_by_value_impl(KeyOfValue kov, Compare comp, ValueOut) const {
        const Value* pv = values;
        std::pair<const Value*, const Value*> ii = std::equal_range(pv, pv + nNodes, kov, comp);
        return std::pair<size_t, size_t>(ii.first - pv, ii.second - pv);
    }
    template<class KeyOfValue, class Compare>
    std::pair<size_t, size_t>
    equal_range_by_value_impl(KeyOfValue kov, Compare comp, ValueInline) const {
        const Node* pn = pNodes;
        std::pair<const Node*, const Node*> ii =
        std::equal_range(pn, pn + nNodes, kov, FindCompareValueInline<KeyOfValue, Compare>(comp));
        return std::pair<size_t, size_t>(ii.first - pn, ii.second - pn);
    }
public:
    template<class ValueCompare>
    void sort_by_value(ValueCompare comp) {
        BOOST_STATIC_ASSERT(!is_value_empty);
        if (nDeleted)
            revoke_deleted_no_relink();
        if (nNodes >= 2) {
            save_strlen_to_link();
            sort_by_value_impl(comp, ValuePlace(), sort_by_index());
        }
        sort_flag = en_sort_by_val;
    }
    void sort_by_value() {
        BOOST_STATIC_ASSERT(!is_value_empty);
        sort_by_value(std::less<Value>());
    }
    template<class KeyOfValue, class Compare>
    size_t lower_bound_by_value(const KeyOfValue& kov, Compare comp) const {
        assert(en_sort_by_val == sort_flag);
        return lower_bound_by_value_impl(kov, comp, ValuePlace());
    }
    template<class KeyOfValue, class Compare>
    size_t upper_bound_by_value(const KeyOfValue& kov, Compare comp) const {
        assert(en_sort_by_val == sort_flag);
        return upper_bound_by_value_impl(kov, comp, ValuePlace());
    }
    template<class KeyOfValue, class Compare>
    std::pair<size_t, size_t>
    equal_range_by_value(const KeyOfValue& kov, Compare comp) const {
        assert(en_sort_by_val == sort_flag);
        return equal_range_by_value_impl(kov, comp, ValuePlace());
    }

    size_t lower_bound_by_value(const Value& val) const {
        assert(en_sort_by_val == sort_flag);
        return lower_bound_by_value_impl(val, std::less<Value>(), ValuePlace());
    }
    size_t upper_bound_by_value(const Value& val) const {
        assert(en_sort_by_val == sort_flag);
        return upper_bound_by_value_impl(val, std::less<Value>(), ValuePlace());
    }
    std::pair<size_t, size_t>
    equal_range_by_value(const Value& val) const {
        assert(en_sort_by_val == sort_flag);
        return equal_range_by_value_impl(val, std::less<Value>(), ValuePlace());
    }

// sort by composite key-value, code is somewhat longer, but simple
// k3v2 : first 3-way key compare, second 2-way val compare
// v2k2 : first 2-way val compare, second 2-way key compare
//
private:
// key first, value second, 3-way key compare
    template<class KeyCmp3, class ValCmp2>
    struct cmp_by_index_ValueOut_k3v2 { // such as case insensitive compare
        bool operator()(const KeyIndex& x, const KeyIndex& y) const {
            const fstring sx(ps + LOAD_OFFSET(x.offset), x.length);
            const fstring sy(ps + LOAD_OFFSET(y.offset), y.length);
            int ret = kc(sx, sy);
            if (ret != 0)
                return ret < 0;
            return vc(pv[x.idx], pv[y.idx]);
        }
        cmp_by_index_ValueOut_k3v2(const char* ps, const Value* pv, KeyCmp3 kc, ValCmp2 vc)
            : ps(ps), pv(pv), kc(kc), vc(vc) {}
        const char*  ps;
        const Value* pv;
        KeyCmp3      kc;
        ValCmp2      vc;
    };
    template<class KeyCmp3, class ValCmp2>
    struct cmp_by_index_ValueInline_k3v2 {
        bool operator()(const KeyIndex& x, const KeyIndex& y) const {
            const fstring sx(ps + LOAD_OFFSET(x.offset), x.length);
            const fstring sy(ps + LOAD_OFFSET(y.offset), y.length);
            int ret = kc(sx, sy);
            if (ret != 0)
                return ret < 0;
            return pn[x.idx].value < pn[y.idx].value;
        }
        cmp_by_index_ValueInline_k3v2(const char* ps, const Node* pn, KeyCmp3 kc, ValCmp2 vc)
            : ps(ps), pn(pn), kc(kc), vc(vc) {}
        const char* ps;
        const Node* pn;
        KeyCmp3     kc;
        ValCmp2     vc;
    };
    template<class KeyCmp3, class ValCmp2>
    struct cmp_ValueInline_k3v2 {
        bool operator()(const Node& x, const Node& y) const {
            fstring sx(ps + LOAD_OFFSET(x.offset), x.link); // link is length
            fstring sy(ps + LOAD_OFFSET(y.offset), y.link);
            int ret = kc(sx, sy);
            if (ret != 0)
                return ret < 0;
            return vc(x.value, y.value);
        }
        cmp_ValueInline_k3v2(const char* ps, KeyCmp3 kc, ValCmp2 vc)
            : ps(ps), kc(kc), vc(vc) {}
        const char* ps;
        KeyCmp3     kc;
        ValCmp2     vc;
    };
    template<class KeyCmp3, class ValCmp2>
    void sort_k3v2_(KeyCmp3 kc, ValCmp2 vc, sort_by_index_yes, ValueOut) {
        KeyIndex* index = buildindex();
        std::sort(index, index + nNodes,
            cmp_by_index_ValueOut_k3v2<KeyCmp3, ValCmp2>(strpool, values, kc, vc));
        rearrange_nodes(index);
        rearrange_strpool(); // must after rearrange_nodes
        relink();
    }
    template<class KeyCmp3, class ValCmp2>
    void sort_k3v2_(KeyCmp3 kc, ValCmp2 vc, sort_by_index_yes, ValueInline) {
        KeyIndex* index = buildindex();
        std::sort(index, index + nNodes,
            cmp_by_index_ValueInline_k3v2<KeyCmp3, ValCmp2>(strpool, pNodes, kc, vc));
        rearrange_nodes(index);
        rearrange_strpool(); // must after rearrange_nodes
        relink();
    }
    template<class KeyCmp3, class ValCmp2>
    void sort_k3v2_(KeyCmp3 kc, ValCmp2 vc,	sort_by_index_no, ValueInline) {
        save_strlen_to_link();
        std::sort(pNodes, pNodes + nNodes, cmp_ValueInline_k3v2<KeyCmp3, ValCmp2>(strpool, kc, vc));
        rearrange_strpool(); // must after rearrange_nodes
        relink_fill();
    }
public:
    template<class KeyCmp3, class ValCmp2>
    void sort_k3v2(KeyCmp3 kc, ValCmp2 vc) {
        BOOST_STATIC_ASSERT(!is_value_empty);
        if (nDeleted)
            revoke_deleted_no_relink();
        if (nNodes >= 2) {
            sort_k3v2_(kc, vc, sort_by_index(), ValuePlace());
        }
        sort_flag = en_sort_by_key;
    }

private:
// value first, key second, 2-way key compare, 2-way value compare
    template<class KeyCmp2, class ValCmp2>
    struct cmp_by_index_ValueOut_v2k2 {
        bool operator()(const KeyIndex& x, const KeyIndex& y) const {
            const Value& vx = pv[x.idx];
            const Value& vy = pv[y.idx];
            if (vc(vx, vy))
                return true;
            else if (vc(vy, vx))
                return false;
            else {
                const fstring sx(ps + LOAD_OFFSET(x.offset), x.length);
                const fstring sy(ps + LOAD_OFFSET(y.offset), y.length);
                return kc(sx, sy);
            }
        }
        cmp_by_index_ValueOut_v2k2(const char* ps, const Value* pv, KeyCmp2 kc, ValCmp2 vc)
            : ps(ps), pv(pv), kc(kc), vc(vc) {}
        const char*  ps;
        const Value* pv;
        KeyCmp2      kc;
        ValCmp2      vc;
    };
    template<class KeyCmp2, class ValCmp2>
    struct cmp_by_index_ValueInline_v2k2 {
        bool operator()(const KeyIndex& x, const KeyIndex& y) const {
            const Value& vx = pn[x.idx].value;
            const Value& vy = pn[y.idx].value;
            if (vc(vx, vy))
                return true;
            else if (vc(vy, vx))
                return false;
            else {
                const fstring sx(ps + LOAD_OFFSET(x.offset), x.length);
                const fstring sy(ps + LOAD_OFFSET(y.offset), y.length);
                return kc(sx, sy);
            }
        }
        cmp_by_index_ValueInline_v2k2(const char* ps, const Node* pn, KeyCmp2 kc, ValCmp2 vc)
            : ps(ps), pn(pn), kc(kc), vc(vc) {}
        const char* ps;
        const Node* pn;
        KeyCmp2     kc;
        ValCmp2     vc;
    };
    template<class KeyCmp2, class ValCmp2>
    struct cmp_ValueInline_v2k2 {
        bool operator()(const Node& x, const Node& y) const {
            if (vc(x.value, y.value))
                return true;
            else if (vc(y.value, x.value))
                return false;
            else {
                const fstring sx(ps + LOAD_OFFSET(x.offset), x.link);
                const fstring sy(ps + LOAD_OFFSET(y.offset), y.link);
                return kc(sx, sy);
            }
        }
        cmp_ValueInline_v2k2(const char* ps, KeyCmp2 kc, ValCmp2 vc)
            : ps(ps), kc(kc), vc(vc) {}
        const char* ps;
        KeyCmp2     kc;
        ValCmp2     vc;
    };
    template<class KeyCmp2, class ValCmp2>
    void sort_v2k2_(KeyCmp2 kc, ValCmp2 vc, sort_by_index_yes, ValueOut) {
        KeyIndex* index = buildindex();
        std::sort(index, index + nNodes,
            cmp_by_index_ValueOut_v2k2<KeyCmp2, ValCmp2>(strpool, values, kc, vc));
        rearrange_nodes(index);
        rearrange_strpool(); // must after rearrange_nodes
        relink();
    }
    template<class KeyCmp2, class ValCmp2>
    void sort_v2k2_(KeyCmp2 kc, ValCmp2 vc, sort_by_index_yes, ValueInline) {
        KeyIndex* index = NULL;
        try { index = buildindex(); } catch (const std::bad_alloc&) {}
        if (index) {
            std::sort(index, index + nNodes,
                cmp_by_index_ValueInline_v2k2<KeyCmp2, ValCmp2>(strpool, pNodes, kc, vc));
            rearrange_nodes(index);
            rearrange_strpool(); // must after rearrange_nodes
            relink();
        }
        else // alloc fail, fall back to in-place sort
            sort_v2k2_(kc, vc, sort_by_index_no(), ValueInline());
    }
    template<class KeyCmp2, class ValCmp2>
    void sort_v2k2_(KeyCmp2 kc, ValCmp2 vc,	sort_by_index_no, ValueInline) {
        save_strlen_to_link();
        std::sort(pNodes, pNodes + nNodes, cmp_ValueInline_v2k2<KeyCmp2, ValCmp2>(strpool, kc, vc));
        rearrange_strpool(); // must after rearrange_nodes
        relink_fill();
    }
public:
    template<class KeyCmp2, class ValCmp2>
    void sort_v2k2(ValCmp2 vc, KeyCmp2 kc) {
        BOOST_STATIC_ASSERT(!is_value_empty);
        if (nDeleted)
            revoke_deleted_no_relink();
        if (nNodes >= 2) {
            sort_v2k2_(kc, vc, sort_by_index(), ValuePlace());
        }
        sort_flag = en_sort_by_val;
    }
    template<class ValCmp2>
    void sort_v2k2(ValCmp2 vc) {
        sort_v2k2(vc, fstring::IF_SP_ALIGN(less_align, less_unalign)());
    }
    void sort_v2k2() {
        sort_v2k2(std::less<Value>(), fstring::IF_SP_ALIGN(less_align, less_unalign)());
    }
};

///////////////////////////////////////////////////////////////////////////////

namespace std { // for std::swap

template< class Value
        , class HashFunc
        , class KeyEqual
        , class ValuePlace
        , class CopyStrategy
        , class LinkTp
        >
void
swap(hash_strmap<Value, HashFunc, KeyEqual, ValuePlace, CopyStrategy, LinkTp> &x,
     hash_strmap<Value, HashFunc, KeyEqual, ValuePlace, CopyStrategy, LinkTp> &y)
{
    x.swap(y);
}

} // namespace std


#endif // __febird_fast_hash_strmap2__
