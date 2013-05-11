#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <types.h>
#include <limits.h>
#include <string.h>
#include <boost/shared_ptr.hpp>

#ifndef IZENELIB_UTIL_MEM_POOL_H_
#define IZENELIB_UTIL_MEM_POOL_H_

namespace izenelib
{
namespace util
{
/**
 * Index segment of a mem_pool has 2 pages, 255 segments at max
 * that is about 64GB with 256M segment
 */
static const size_t SEG_INDEX_SIZE=8192;

/**
 * Default segment size is 256M
 */
static const size_t DEFAULT_SEG_SIZE=64*1024*1024;

/**
 * Default anonymous mem pool segment size is 4M
 */
static const size_t DEFAULT_ANON_SEG_SIZE=4*1024*1024;

namespace detail
{
template<int N>
struct TYPE_SIZE
{
    typedef uint32_t OFFSET;
};

template<>
struct TYPE_SIZE< 8 >
{
    typedef uint64_t OFFSET;
};

template<>
struct TYPE_SIZE< 4 >
{
    typedef uint32_t OFFSET;
};

typedef const char *charptr_t;

/**
 * Return an aligned segment size, which is 2^n
 *
 * @param suggested_min_seg_size suggested minimal size of the segment
 * @return An aligned segment size to create mem_pool, this value is not less then suggested_min_seg_size
 */
size_t get_aligned_seg_size(size_t suggested_min_seg_size=DEFAULT_SEG_SIZE);
}

typedef detail::TYPE_SIZE< sizeof(detail::charptr_t) >::OFFSET OFFSET;
static const OFFSET INVALID_OFFSET =(OFFSET)-1;

class mem_pool;

/**
 * Template offptr_t convert OFFSET to typed pointer or array
 */
template<typename T>
struct offptr_t
{
    typedef offptr_t<T> this_t;
    typedef T element_type;

    offptr_t()
        : pool(0), offset(INVALID_OFFSET)
    {}

    offptr_t(const mem_pool *p, OFFSET o=INVALID_OFFSET)
        : pool(p), offset(o)
    {}

    offptr_t(const this_t &other)
        : pool(other.pool), offset(other.offset)
    {}

    offptr_t &operator=(const this_t &other)
    {
        pool=other.pool;
        offset=other.offset;
        return *this;
    }

    bool operator==(const this_t &other) const
    {
        return pool==other.pool && offset==other.offset;
    }

    bool operator!=(const this_t &other) const
    {
        return !(*this==other);
    }

    operator bool() const
    {
        return pool && offset!=INVALID_OFFSET;
    }

    T& operator*()
    {
        return *get();
    }

    const T& operator*() const
    {
        return *get();
    }

    T& operator[](size_t index);

    const T& operator[](size_t index) const;

    T* operator->()
    {
        return get();
    }

    const T* operator->() const
    {
        return get();
    }

    this_t operator+(size_t v)
    {
        return this_t(pool, offset+sizeof(T)*v);
    }

    this_t operator-(size_t v)
    {
        return this_t(pool, offset-sizeof(T)*v);
    }

    int64_t operator-(const this_t &v)
    {
        return (offset-v.offset)/sizeof(T);
    }

    this_t operator+=(size_t v)
    {
        offset+=-sizeof(T)*v;
        return this_t(pool, offset);
    }

    this_t operator-=(size_t v)
    {
        offset-=sizeof(T)*v;
        return this_t(pool, offset);
    }

    this_t &operator++()
    {
        offset+=sizeof(T);
        return *this;
    }

    this_t operator++(int)
    {
        offset+=sizeof(T);
        return this_t(pool, offset);
    }

    this_t &operator--()
    {
        offset-=sizeof(T);
        return *this;
    }

    this_t operator--(int)
    {
        offset-=sizeof(T);
        return this_t(pool, offset);
    }

    T *get() const;

    OFFSET get_offset() const
    {
        return offset;
    }

    const mem_pool *get_pool() const
    {
        return pool;
    }

    const mem_pool *pool;
    OFFSET offset;
};

/**
 * mem_pool class manages a set of mmap, which can be expended as needed
 */
class mem_pool
{
public:
    /**
     * Constructor that initializes a new mem_pool, with or without name,
     *
     * @param seg_size Size of each segment
     * @param name Name of underlying file, create anonymous mapping if it's NULL
     */
    mem_pool(size_t seg_size=0, const char *name=NULL);

    /**
     * Constructor that loads an existing mem_pool, or create pool with default seg size
     *
     * @param name Name of underlying file, create anonymous mapping if it's NULL
     */
    mem_pool(const char *name);

    /**
     * Constructor that loads an existing mem_pool, or create pool with default seg size
     *
     * @param name Base name of underlying file, create anonymous mapping if it's NULL
     * @param name Suffix of underlying file, use base_name if it's NULL
     */
    mem_pool(const char *base_name, const char *suffix);

    /**
     * Constructor that loads an existing mem_pool, or create pool with default seg size
     *
     * @param name Base name of underlying file, create anonymous mapping if it's NULL
     * @param name Suffix of underlying file, use base_name if it's NULL
     */
    mem_pool(size_t seg_size, const char *base_name, const char *suffix);

    /**
     * Destructor
     */
    ~mem_pool();

    /**
     * Initialize a new pool with segment size
     */
    bool init(size_t seg_size);

    /**
     * Close and clear currently opened pool, pool can be reopened afterward
     */
    void clear();

    /**
     * Reset content, this will destroy on-disk file
     *
     * After reset, all segments are destroyed, segment size remains
     */
    void reset();

    /**
     * Load existing pool
     */
    bool load();

    /**
     * Save pool content to disk
     */
    bool save();

    /**
     * Expand the pool, make sure it's size is at least 'size' bytes
     *
     * @param size Expected size after the expansion, 0 to expand one segment
     */
    bool expand(size_t size=0);

    /**
     * Check the capacity of the pool
     *
     * @return Size of the pool in byte
     */
    inline size_t capacity() const
    {
        return get_index()->used_seg * get_index()->seg_size;
    }

    /**
     * Convert OFFSET to memory address
     *
     * The returned address should *not* be saved for later use as it may change
     *
     * @param offset the offset in the pool
     * @return memory address
     */
    inline void *get_addr(OFFSET offset) const
    {
        size_t seg_idx=offset / get_index()->seg_size;
        if (seg_idx>=get_index()->used_seg)
        {
            return NULL;
        }
        OFFSET seg_off=offset % get_index()->seg_size;
        return get_seg(int(seg_idx)).get_addr(seg_off, segment_bases_[seg_idx]);
    }

    /**
     * Convert OFFSET to typed pointer, offptr_t<T>
     *
     * The returned offptr_t can be saved for later use as it doesn't contain absolute address
     *
     * @param offset the offset in the pool
     * @return typed offptr_t
     */
    template<typename T>
    inline offptr_t<T> offptr(OFFSET offset) const
    {
        return offptr_t<T>(this, offset);
    }

    /**
     * Allocate memory from the pool
     *
     * NOTE: the length is *not* automatically saved, there is no way to retrieve the length
     * unless you save it manually
     *
     * @param length length of source data
     * @return the offset which allocated space exists in the pool
     */
    OFFSET allocate(size_t length);

    /**
     * Allocate n of T from the pool
     *
     * @param n number of T to allocate
     * @return the offptr_t<T> points to the first allocated element
     */
    template<typename T>
    offptr_t<T> allocate(size_t n=1)
    {
        return offptr<T>(allocate(n*sizeof(T)));
    }

    /**
     * Add a piece of data into the pool
     *
     * NOTE: the length is *not* automatically saved, there is no way to retrieve the length
     * unless you save it manually
     *
     * @param p pointer to source data
     * @param length length of source data
     * @return the offset which newly added data exists in the pool
     */
    OFFSET add_chunk(const void *p, size_t length);

    /**
     * Add a piece of data into the pool
     *
     * NOTE: the length is *not* automatically saved, there is no way to retrieve the length
     * unless you save it manually
     *
     * @param p pointer to source data
     * @param length length of source data
     * @return the offset which newly added data exists in the pool
     */
    OFFSET add_chunk_with_length(const void *p, size_t length);

    /**
     * Add a string into the pool
     *
     * NOTE: the length is *not* automatically saved, but the trailing '\0' is stored
     * so you can use string functions once you convert OFFSET to pointers
     *
     * @param p pointer to source string
     * @return the offset which newly added string exists in the pool
     */
    inline OFFSET add_string(const char *p)
    {
        // Include trailing '\0'
        return add_chunk(p, strlen(p)+1);
    }

    /**
     * Add a string into the pool
     *
     * NOTE: the length is *not* automatically saved, but the trailing '\0' is stored
     * so you can use string functions once you convert OFFSET to pointers
     *
     * @param p pointer to source string
     * @return the offset which newly added string exists in the pool
     */
    inline OFFSET add_string(const char *p, size_t sz)
    {
        // Add trailing '\0'
        OFFSET ret=add_chunk(p, sz+1);
        offptr<char>(ret)[sz]='\0';
        return ret;
    }

    /**
     * Add a string into the pool
     *
     * NOTE: the length is *not* automatically saved, but the trailing '\0' is stored
     * so you can use string functions once you convert OFFSET to pointers
     *
     * @param p pointer to source string
     * @return the offset which newly added string exists in the pool
     */
    inline OFFSET add_string_with_length(const char *p)
    {
        // Include trailing '\0'
        return add_chunk_with_length(p, strlen(p)+1);
    }

    /**
     * Return the name of the mem_pool, which can be name of a on-disk file
     */
    inline const char *get_name() const
    {
        return name_;
    }

    /**
     * Rename underlying file
     *
     * There could be race condition between rename and expand, make sure
     * they're not called simultaneously from different threads
     *
     * @param new_name new name of underlying file
     * @return true if success, otherwise false
     */
    bool rename(const char *new_name);

    /**
     * Return used memory of the mem_pool, in bytes
     *
     * This size is *not* accurate and should be only used in statistics.
     */
    inline size_t get_used_size() const
    {
        return SEG_INDEX_SIZE   // Size of index segment
               +(get_index()->used_seg-1)*get_index()->seg_size    // Size of all full segments
               +get_seg(int(get_index()->used_seg)-1).used_size_;       // Used size of last segment
    }

    /*
     * Return size of mapping segment
     */
    inline size_t get_segment_size() const
    {
        return get_index()->seg_size;
    }

private:
    struct segment
    {
        inline void *get_addr(OFFSET o, void *base) const
        {
            if(!base) return NULL;
            return ((char *)base)+o;
        }

        inline OFFSET add_chunk(const void *p, size_t length, void *base)
        {
            OFFSET ret=used_size_;
            memcpy(((char *)base)+used_size_, p, length);
            //used_size_+=length;
            return ret;
        }

        inline OFFSET allocate(size_t length)
        {
            OFFSET ret=used_size_;
            //used_size_+=length;
            return ret;
        }

        void *init(int fd, size_t offset, size_t size);

        inline void clear()
        {
            // This is a POD
            memset(this, 0, sizeof(segment));
        }

        inline bool check(void *base) const
        {
            return base!=NULL;
        }

        size_t used_size_;
        //void *base_;
    };

    struct col_idx
    {
        size_t used_seg;
        size_t seg_size;
        segment segments[10];
    };

    inline size_t avail_size(int index) const
    {
        return get_index()->seg_size-get_seg(index).used_size_;
    }

    inline col_idx *get_index() const
    {
        return idx_;
    }

    inline segment &get_seg(int index) const
    {
        return get_index()->segments[index];
    }

    inline OFFSET map_offset_seg_to_pool(int index, OFFSET offset=0) const
    {
        return offset+index*get_index()->seg_size;
    }

    inline size_t get_seg_offset(int index) const
    {
        if (index<0)
        {
            return 0;
        }
        return SEG_INDEX_SIZE+index*actual_seg_size_;
    }

    int find_avail_seg(size_t size) const;
    bool open_idx(bool initialize);
    bool new_segment();
    bool open_segment(int index, segment *seg, size_t size, bool initialize=false);
    bool set_file_size(int fd, size_t sz, bool may_shrink);

private:
    char name_[PATH_MAX];
    segment idx_seg_;
    col_idx *idx_;
    void *segment_bases_[SEG_INDEX_SIZE/sizeof(void *)];
    size_t actual_seg_size_;
};

typedef boost::shared_ptr<mem_pool> mem_pool_ptr_t;

template<typename T>
T *offptr_t<T>::get() const
{
    return (T*)(pool->get_addr(offset));
}

template<typename T>
T& offptr_t<T>::operator[](size_t index)
{
    return *((T *)(pool->get_addr(offset+index*sizeof(T))));
}

template<typename T>
const T& offptr_t<T>::operator[](size_t index) const
{
    return *((const T *)(pool->get_addr(offset+index*sizeof(T))));
}

const char *add_suffix(char *buf, const char *name, const char *suffix);

}
}

#endif
