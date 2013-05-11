#ifndef IZENELIB_UTIL_MEM_POOL_ALLOCATOR_H_
#define IZENELIB_UTIL_MEM_POOL_ALLOCATOR_H_

#include <memory>
#include <string>

#include <boost/type_traits.hpp>
#include "mem_pool.h"

namespace izenelib
{
namespace util
{
/**
 * mem_pool_allocator is a STL conformance allocator.
 *
 * It's backed by a mem_pool and does not support deallocate. User can reset
 * or clear the pool when *all* allocated memory are not in use.
 */
template<typename T>
class mem_pool_allocator
{
public:
    typedef mem_pool_allocator<T> this_t;
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef int64_t difference_type;
    template< class U > struct rebind
    {
        typedef mem_pool_allocator<U> other;
    };

    inline mem_pool_allocator()
        : pool(0)
    {}

    inline mem_pool_allocator(mem_pool &p)
        : pool(&p)
    {}

    inline mem_pool_allocator(mem_pool *p)
        : pool(p)
    {}

    inline mem_pool_allocator(const this_t &a)
        : pool(a.pool)
    {}

    template<typename U>
    inline mem_pool_allocator(const mem_pool_allocator<U>& a)
        : pool(a.pool)
    {}

    /**
     * It's very weird that boost::unordered_map is *not* using rebind, it uses
     * type conversion directly?
     */
    /*
    template<typename U>
    operator mem_pool_allocator<U>() const
    {
        return mem_pool_allocator<U>();
    }
     */

    inline pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
    {
        //std::cout << "Allocate " << n*sizeof(T) << " bytes.\n";
        return (pointer)(pool->get_addr(pool->allocate(n*sizeof(T))));
    }

    inline void deallocate(pointer p, size_type n)
    {
        // Do nothing, we do not support recycling
    }

    inline size_type max_size() const
    {
        // Maximize allocatable block size is segment size
        return pool->get_segment_size();
    }

    inline void construct(pointer p, const T& t)
    {
        // Copy Construct T at p
        new(p) T(t);
    }

    inline void destroy(pointer p)
    {
        // Destructe p
        p->~T();
    }

    inline char *malloc(size_t n)
    {
        return (pointer)(pool->get_addr(pool->allocate(n)));
    }

    inline void free(char *p)
    {
        // Do nothing
    }

    template<typename U>
    inline bool operator==(const mem_pool_allocator<U> & a) const
    {
        return pool==a.pool;
    }

    template<typename U>
    inline bool operator!=(const mem_pool_allocator<U> & a) const
    {
        return !operator==(a);
    }

    // This allocator does need to be propagated on copy/move/swap
    typedef boost::true_type propagate_on_container_copy_assignment;
    typedef boost::true_type propagate_on_container_move_assignment;
    typedef boost::true_type propagate_on_container_swap;

    mem_pool *pool;
};

mem_pool *get_tl_mem_pool();

/**
 * This allocator is actually a workaround for C++03
 *
 * STL containers do not copy allocators during construction/copying/swapping etc,
 * In stead they default-construct allocator when needed, that means they do not
 * support stateful allocator.
 * All allocators work with STL containers must be default constructable and all
 * constructed instance must work with same underlying memory managements.
 * In C++11, the mem_pool_allocator should work if I finish a proper allocator_traits
 * class, this will be done when I've enough knowledge.
 */
template<typename T>
struct mem_pool_tl_allocator : public mem_pool_allocator<T>
{
    typedef mem_pool_allocator<T> super_t;
    typedef mem_pool_tl_allocator<T> this_t;
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef int64_t difference_type;
    template<class U> struct rebind
    {
        typedef mem_pool_tl_allocator<U> other;
    };

    mem_pool_tl_allocator()
        : super_t(get_tl_mem_pool())
    {}

    inline pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
    {
        //std::cout << "Allocate " << n*sizeof(T) << " bytes.\n";
        return (pointer)(super_t::pool->get_addr(super_t::pool->allocate(n*sizeof(T))));
    }

    /**
     * It's very weird that boost::unordered_map is *not* using rebind, it uses
     * type conversion directly?
     */
    template<typename U>
    operator mem_pool_tl_allocator<U>() const
    {
        return mem_pool_tl_allocator<U>();
    }
};

template<typename T>
struct mem_pool_deallocator
{
    // mem_pool_allocator::deallocate does nothing anyway
    void operator()(T*p) {}
};

/**
 * Predefined allocator for std::basic_string
 */
typedef mem_pool_allocator<char> mps_allocator;

/**
 * mem_pool backed std::basic_string, defined here for convenience
 */
typedef std::basic_string<char, std::char_traits<char>, mps_allocator> mpstring;

/**
 * Predefined thread-local allocator for std::basic_string
 */
typedef mem_pool_tl_allocator<char> mps_tl_allocator;

/**
 * mem_pool backed std::basic_string, defined here for convenience
 */
typedef std::basic_string<char, std::char_traits<char>, mps_tl_allocator> mptlstring;

//typedef mptlstring arstring;
typedef std::string arstring;
}
}

template<typename T>
inline void *operator new(size_t sz, izenelib::util::mem_pool_allocator<T> allocator)
{
    return allocator.allocate(sz);
}

template<typename T>
inline void *operator new[](size_t sz, izenelib::util::mem_pool_allocator<T> allocator)
{
    return allocator.allocate(sz);
}

template<typename T>
inline void *operator new(size_t sz, izenelib::util::mem_pool_tl_allocator<T> allocator)
{
    return allocator.allocate(sz);
}

template<typename T>
inline void *operator new[](size_t sz, izenelib::util::mem_pool_tl_allocator<T> allocator)
{
    return allocator.allocate(sz);
}

#endif
