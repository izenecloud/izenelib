#include <util/MemPool.h>

namespace izenelib{namespace util{

#define DEFAULT_EM_POOL_SIZE 1024*1024

/////////////////////////////////////////////////////
/// MemPool
MemPool::EmergencyPool::EmergencyPool(size_t nInitSize)
        : nSliceSize_(nInitSize)
{
    try
    {
        void* p = new uint8_t[nInitSize + sizeof(Slice)];
        pTailSlice_ = pHeadSlice_ = new(p) Slice(nInitSize);
        nUpto_ = 0;
    }
    catch (std::bad_alloc& )
    {
        throw std::runtime_error("Allocate memory FAILED.");
    }
}

MemPool::EmergencyPool::~EmergencyPool()
{
    MemPool::EmergencyPool::Slice* pSlice = pHeadSlice_;
    MemPool::EmergencyPool::Slice* pTmpSlice = NULL;
    while (pSlice)
    {
        pTmpSlice = pSlice->next;
        delete[] (uint8_t*)pSlice;
        pSlice = pTmpSlice;
    }
    pTailSlice_ = pHeadSlice_ = NULL;
}

/////////////////////////////////////////////////////////////////////////
///
MemPool::MemPool(size_t nPoolSize)
        : nBufSize_(nPoolSize)
        , nEMPoolSize_(DEFAULT_EM_POOL_SIZE)
        , pEMPool_(NULL)
{
    try
    {
        pUpto_ = pBuffer_ = new uint8_t[nPoolSize];
    }
    catch (std::bad_alloc& )
    {
        throw std::runtime_error("Allocate memory FAILED.");
    }

}

MemPool::~MemPool()
{
    if (pBuffer_)
    {
        delete[] (uint8_t*)pBuffer_;
    }
    pBuffer_ = NULL;
    if (pEMPool_)
    {
        delete pEMPool_;
        pEMPool_ = NULL;
    }
}

void MemPool::Reset()
{
    pUpto_ = pBuffer_;
    if (pEMPool_)
    {
        delete pEMPool_;
        pEMPool_ = NULL;
    }
}


/////////////////////////////////////////////////////
/// Arena
static const int kBlockSize = 4096;

Arena::Arena()
{
    blocks_memory_ = 0;
    alloc_ptr_ = NULL;  // First allocation will allocate a block
    alloc_bytes_remaining_ = 0;
}

Arena::~Arena()
{
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        delete[] blocks_[i];
    }
}

char* Arena::AllocateFallback(size_t bytes)
{
    if (bytes > kBlockSize / 4)
    {
        // Object is more than a quarter of our block size.  Allocate it separately
        // to avoid wasting too much space in leftover bytes.
        char* result = AllocateNewBlock(bytes);
        return result;
    }

    // We waste the remaining space in the current block.
    alloc_ptr_ = AllocateNewBlock(kBlockSize);
    alloc_bytes_remaining_ = kBlockSize;

    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
}

char* Arena::AllocateAligned(size_t bytes)
{
    const int align = sizeof(void*);    // We'll align to pointer size
    assert((align & (align-1)) == 0);   // Pointer size should be a power of 2
    size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align-1);
    size_t slop = (current_mod == 0 ? 0 : align - current_mod);
    size_t needed = bytes + slop;
    char* result;
    if (needed <= alloc_bytes_remaining_)
    {
        result = alloc_ptr_ + slop;
        alloc_ptr_ += needed;
        alloc_bytes_remaining_ -= needed;
    }
    else
    {
        // AllocateFallback always returned aligned memory
        result = AllocateFallback(bytes);
    }
    assert((reinterpret_cast<uintptr_t>(result) & (align-1)) == 0);
    return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes)
{
    char* result = new char[block_bytes];
    blocks_memory_ += block_bytes;
    blocks_.push_back(result);
    return result;
}


}}


