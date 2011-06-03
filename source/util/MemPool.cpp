#include <util/MemPool.h>

namespace izenelib{ namespace util{

#define DEFAULT_EM_POOL_SIZE 1024*1024
#define DEFAULT_EM_POOL_CHUNK_SIZE 32

/////////////////////////////////////////////////////
///
MemCache::EmergencyPool::EmergencyPool(size_t nInitSize)
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

MemCache::EmergencyPool::~EmergencyPool()
{
    MemCache::EmergencyPool::Slice* pSlice = pHeadSlice_;
    MemCache::EmergencyPool::Slice* pTmpSlice = NULL;
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
MemCache::MemCache(size_t nPoolSize)
        : nBufSize_(nPoolSize)
        , nEMPoolSize_(DEFAULT_EM_POOL_SIZE)
        , nEMAllocSize_(DEFAULT_EM_POOL_CHUNK_SIZE)
        , pEMPool_(NULL)
        , bMemOwner_(true)
{
    try
    {
        pUpto_ = pBuffer_ = new uint8_t[nPoolSize];
    }
    catch (std::bad_alloc& )
    {
        throw std::runtime_error("Allocate memory FAILED.");
    }

    if (nEMAllocSize_ < 32)
    {
        nEMAllocSize_ = 32;
    }
}

MemCache::~MemCache()
{
    if (bMemOwner_ && pBuffer_)
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

void MemCache::reset()
{
    pUpto_ = pBuffer_;
    if (pEMPool_)
    {
        delete pEMPool_;
        pEMPool_ = NULL;
    }
}

}}

