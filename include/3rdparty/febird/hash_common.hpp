#ifndef __febird_fast_hash_common__
#define __febird_fast_hash_common__

//#include <stddef.h> // part of C99, not available in all compiler
//#include <stdint.h>

#include <sys/types.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <memory>

//#include <tr1/type_traits>
#include <boost/static_assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

// HSM_ : Hash String Map
#define HSM_SANITY assert

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    #define HSM_HAS_MOVE
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
    #define HSM_unlikely(expr) __builtin_expect(expr, 0)
#else
    #define HSM_unlikely(expr) expr
#endif

inline static size_t
__hsm_stl_next_prime(size_t __n)
{
    static const size_t primes[] =
    {
        5ul,          53ul,         97ul,         193ul,       389ul,
        769ul,        1543ul,       3079ul,       6151ul,      12289ul,
        24593ul,      49157ul,      98317ul,      196613ul,    393241ul,
        786433ul,     1572869ul,    3145739ul,    6291469ul,   12582917ul,
        25165843ul,   50331653ul,   100663319ul,  201326611ul, 402653189ul,
        805306457ul,  1610612741ul, 3221225473ul, 4294967291ul
    };
    const size_t* __first = primes;
    const size_t* __last = primes + sizeof(primes)/sizeof(primes[0]);
    const size_t* pos = std::lower_bound(__first, __last, __n);
    return pos == __last ? __last[-1] : *pos;
}

inline size_t __hsm_align_pow2(size_t x) {
    assert(x > 0);
    size_t p = 0, y = x;
    while (y)
        y >>= 1, ++p;
    if ((size_t(1) << (p-1)) == x)
        return x;
    else
        return size_t(1) << p;
}

template<class UInt>
struct dummy_bucket{
    static const UInt tail = ~UInt(0);
    static UInt b[1];
};
template<class UInt> UInt dummy_bucket<UInt>::b[1] = {~UInt(0)};

//************************************************************************
// std components that could use FastCopy:
//     std::vector
//     std::string
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// some implementation of std components use pointers to embedded objects,
// so you shouldn't use FastCopy for it, because there is no a type traits
// for memcpy-able, and for performance issue, this lib will use FastCopy
// by default, if some class couldn't be memcpy-able, you must use SafeCopy
//     std::list
//     std::*stream, all iostream component shouldn't use FastCopy
//     std::map/set/multimap/multiset couldn't use FastCopy
// std components that couldn't use FastCopy:
// anything you are not sure be memcpy-able shouldn't use FastCopy
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
struct SafeCopy {};
struct FastCopy {};

struct ValueInline {};
struct ValueOut {};

#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
    #define HSM_FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define HSM_FORCE_INLINE __forceinline
#else
    #define HSM_FORCE_INLINE inline
#endif

#endif // __febird_fast_hash_common__
