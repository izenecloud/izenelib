#ifndef IZENELIB_UTIL_MEMPOOL_H
#define IZENELIB_UTIL_MEMPOOL_H

#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/pool/pool.hpp>

#include <3rdparty/boost/lockfree/fifo.hpp>
#include <util/singleton.h>

#include <types.h>
#include <iostream>
#include <new>
#include <vector>

namespace izenelib{namespace util{

/// MemCache: a memory pool not for allocator usage, non-thread-safe, with emergency mechanism
class MemCache
{
public:
    class EmergencyPool
    {
        struct Slice
        {
            Slice(size_t n) : size(n), next(NULL) {}
            size_t size;
            Slice* next;
            uint8_t buffer[1];
        };

    public:
        EmergencyPool(size_t nInitSize);

        ~EmergencyPool();

        inline void* Allocate(size_t nMemSize)
        {
            if (nUpto_ + nMemSize > pTailSlice_->size)
                NewSlice(nMemSize);
            void* pMem = &(pTailSlice_->buffer[nUpto_]);
            nUpto_ += nMemSize;
            return pMem;
        }

    protected:
        inline void NewSlice(size_t nMinSize)
        {
            size_t size = nSliceSize_;
            if (size < nMinSize)
            {
                size = nMinSize;
            }
            void* p = new uint8_t[size + sizeof(Slice)];
            Slice* pSlice = new(p) Slice(size);
            pTailSlice_->next = pSlice;
            pTailSlice_ = pSlice;
            nUpto_ = 0;
        }

    protected:
        Slice* pHeadSlice_; ///head slice
        Slice* pTailSlice_;	///current slice
        size_t nUpto_; ///start address of free space in current slice
        size_t nSliceSize_;
    };

public:
    /**
     * @param nPoolSize memory pool size
     */
    MemCache(size_t nPoolSize);

    ~MemCache();

public:
    /**
     * @param nMemSize size of memory
     * @return allocated memroy address, if the main memory block is exhausted,
     * it will be allocated from emergency block.
     */
    inline void* Allocate(size_t nMemSize)
    {
        if ((pUpto_ - pBuffer_) + nMemSize > nBufSize_)
        {
            return AllocateFromEMPool(nMemSize);
        }
        void* pMem = pUpto_;
        pUpto_ += nMemSize;
        return pMem;
    }

    /**
     * get the begin of mem pool
     */
    inline void* GetBegin() const
    {
        return pBuffer_;
    }

    /**
     * get the end of free mem poool
     */
    inline void* GetEnd() const
    {
        return pUpto_;
    }

    /**
     * get mem pool size
     */
    inline size_t GetSize() const
    {
        return nBufSize_;
    }

    /**
     * Is memory pool empty
     * @return true for empty
     */
    inline bool IsEmpty() const
    {
        return (pBuffer_ == pUpto_);
    }

    /**
     * Is the mem pool in a state of emergency
     * @return true for yes
     */
    inline bool IsEmergency() const
    {
        return (pEMPool_ != NULL);
    }

    /**
     * Set emergency pool size
     */
    inline void SetEMPoolSize(size_t nEMPoolSize)
    {
        nEMPoolSize_ = nEMPoolSize;
    }

    /**
     * reset for reuse
     */
    void Reset();

protected:
    /**
     * Allocate memory from emergency pool
     * @param request memory size
     * @return memory pointer, its size is <i>nMemSize</i>
     */
    inline void* AllocateFromEMPool(size_t nMemSize)
    {
        if (!pEMPool_)
        {
            pEMPool_ = new EmergencyPool(nEMPoolSize_);
        }
        return pEMPool_->Allocate(nMemSize);
    }

private:
    uint8_t* pBuffer_; /// address of memory pool
    size_t nBufSize_; /// size of memory pool
    uint8_t* pUpto_; /// the start address of free space

    size_t nEMPoolSize_; ///init emergency pool size

    MemCache::EmergencyPool* pEMPool_;

    bool bMemOwner_; /// own the memory pool buffer or not
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/// Arena: a simple memory pool for allocator usage, non-thread-safe
/// Copied from LevelDB
class Arena
{
public:
    Arena();
    ~Arena();

    // Return a pointer to a newly allocated memory block of "bytes" bytes.
    char* Allocate(size_t bytes);

    // Allocate memory with the normal alignment guarantees provided by malloc
    char* AllocateAligned(size_t bytes);

    // Returns an estimate of the total memory usage of data allocated
    // by the arena (including space allocated but not yet used for user
    // allocations).
    size_t MemoryUsage() const
    {
        return blocks_memory_ + blocks_.capacity() * sizeof(char*);
    }

private:
    char* AllocateFallback(size_t bytes);
    char* AllocateNewBlock(size_t block_bytes);

    // Allocation state
    char* alloc_ptr_;
    size_t alloc_bytes_remaining_;

    // Array of new[] allocated memory blocks
    std::vector<char*> blocks_;

    // Bytes of memory in blocks allocated so far
    size_t blocks_memory_;

    // No copying allowed
    Arena(const Arena&);
    void operator=(const Arena&);
};

inline char* Arena::Allocate(size_t bytes)
{
    // The semantics of what to return are a bit messy if we allow
    // 0-byte allocations, so we disallow them here (we don't need
    // them for our internal use).
    assert(bytes > 0);
    if (bytes <= alloc_bytes_remaining_)
    {
        char* result = alloc_ptr_;
        alloc_ptr_ += bytes;
        alloc_bytes_remaining_ -= bytes;
        return result;
    }
    return AllocateFallback(bytes);
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///
/// MemPoolAllocator: a thread-safe, small object allocator that sacrifices
/// memory utilization for performance.  It combines a
/// collection of fixed-size pooled memory allocators with
/// lock-free caches to achieve nearly wait-free, constant
/// time performance when used for an extended period of time
///
template <std::size_t MinSize = 16, std::size_t MaxSize = 256>
class MemPoolAllocator : private boost::noncopyable
{
public:

    /// virtual destructor
    virtual ~MemPoolAllocator()
    {}

    /// default constructor
    MemPoolAllocator(void)
    {
        for (std::size_t n = 0; n < NumberOfAllocs; ++n)
        {
            pools_[n].reset(new FixedSizeAlloc((n+1) * MinSize));
        }
    }

    static MemPoolAllocator<MinSize,MaxSize>* get()
    {
        return ::izenelib::util::Singleton<MemPoolAllocator<MinSize,MaxSize> >::get();
    }

    /**
     * allocates a block of memory
     *
     * @param n minimum size of the new memory block, in bytes
     *
     * @return void * raw pointer to the new memory block
     */
    inline void *malloc(std::size_t n)
    {
        // check for size greater than MaxSize
        if (n > MaxSize)
            return ::malloc(n);
        FixedSizeAlloc *pool_ptr = getPool(n);

        while (true)
        {
            // get copy of free list pointer
            FreeListPtr old_free_ptr(pool_ptr->free_ptr_);
            if (! old_free_ptr)
                break;	// use pool alloc if free list is empty

            // use CAS operation to swap the free list pointer
            if (pool_ptr->free_ptr_.cas(old_free_ptr, old_free_ptr->next.get_ptr()))
                return reinterpret_cast<void*>(old_free_ptr.get_ptr());
        }
        boost::unique_lock<boost::mutex> pool_lock(pool_ptr->mutex_);
        return pool_ptr->pool_.malloc();
    }

    /**
     * deallocates a block of memory
     *
     * @param ptr raw pointer to the block of memory
     * @param n requested size of the memory block, in bytes (actual size may be larger)
     */
    inline void free(void *ptr, std::size_t n)
    {
        // check for size greater than MaxSize
        if (n > MaxSize)
        {
            ::free(ptr);
            return;
        }
        FixedSizeAlloc *pool_ptr = getPool(n);
        while (true)
        {
            // get copy of free list pointer
            FreeListPtr old_free_ptr(pool_ptr->free_ptr_);

            // cast memory being released to a free list node
            // and point its next pointer to the current free list
            FreeListNode *node_ptr = reinterpret_cast<FreeListNode*>(ptr);
            node_ptr->next.set_ptr(old_free_ptr.get_ptr());

            // use CAS operation to swap the free list pointer
            if (pool_ptr->free_ptr_.cas(old_free_ptr, node_ptr))
                break;
        }
        boost::unique_lock<boost::mutex> pool_lock(pool_ptr->mutex_);
        return pool_ptr->pool_.free(ptr);
    }

    /**
     * releases every memory block that does not have any allocated chunks
     */
    inline void release_memory()
    {
#if 0
        ///can not work right now
        for (std::size_t n = 0; n < NumberOfAllocs; ++n)
        {
            FixedSizeAlloc *pool_ptr = pools_[n].get();
            // need to lock before releasing free list because of calls
            // to pool::free()
            boost::unique_lock<boost::mutex> pool_lock(pool_ptr->mutex_);
            while (true)
            {
                // get copy of free list pointer
                FreeListPtr old_free_ptr(pool_ptr->free_ptr_);
                if (! old_free_ptr)
                    break; // all done: free list is empty
                // use CAS operation to swap the free list pointer
                if (pool_ptr->free_ptr_.cas(pool_ptr->free_ptr_, old_free_ptr->next.get_ptr()))
                    pool_ptr->pool_.free(old_free_ptr.get_ptr());	// release memory from pool
            }
        }
#endif
    }


protected:

    /// data structure used to represent a free node
    struct FreeListNode
    {
        boost::lockfree::tagged_ptr<struct FreeListNode> next;
    };

    /// data type for a tagged free-list pointer
    typedef boost::lockfree::tagged_ptr<struct FreeListNode> FreeListPtr;

    /// ensure that:
    ///   a) MaxSize >= MinSize
    ///   b) MaxSize is a multiple of MinSize
    ///   c) MinSize >= sizeof(FreeNodePtr)  [usually 16]
    BOOST_STATIC_ASSERT(MaxSize >= MinSize);
    BOOST_STATIC_ASSERT(MaxSize % MinSize == 0);
    BOOST_STATIC_ASSERT(MinSize >= sizeof(FreeListNode));

    /// constant representing the number of fixed-size pool allocators
    enum { NumberOfAllocs = ((MaxSize-1) / MinSize) + 1 };

    /**
     * data structure used to represent a pooled memory
     * allocator for blocks of a specific size
     */
    struct FixedSizeAlloc
    {
        /**
         * constructs a new fixed-size pool allocator
         *
         * @param size size of memory blocks managed by this allocator, in bytes
         */
        FixedSizeAlloc(std::size_t size)
                : size_(size), pool_(size), free_ptr_(NULL)
        {}

        /// used to protect access to the memory pool
        boost::mutex mutex_;

        /// size of memory blocks managed by this allocator, in bytes
        std::size_t size_;

        /// underlying pool allocator used for memory management
        boost::pool<> pool_;

        /// pointer to a list of free nodes (for lock-free cache)
        FreeListPtr free_ptr_;
    };


    /**
     * gets an appropriate fixed-size pool allocator
     *
     * @param n the number of bytes to be (de)allocated
     *
     * @return FixedSizeAlloc* pointer to the appropriate fixed-size allocator
     */
    inline FixedSizeAlloc* getPool(const std::size_t n)
    {
        BOOST_ASSERT(n > 0);
        BOOST_ASSERT(n <= MaxSize);
        return pools_[ (n-1) / MinSize ].get();
    }

private:

    /// a collection of fixed-size pool allocators
    boost::array<boost::scoped_ptr<FixedSizeAlloc>, NumberOfAllocs>	pools_;
};


}}


#endif

