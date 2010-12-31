#include <ir/index_manager/utility/MemCache.h>

#include <cassert>

using namespace izenelib::ir::indexmanager;

MemCache::MemCache(size_t cachesize)
    :begin_(NULL)
    ,end_(NULL)
    ,size_(cachesize)
    ,pGrowCache_(NULL)
{
    begin_ = (uint8_t*) malloc(size_);
    if (begin_ == NULL)
    {
        SF1V5_THROW(ERROR_OUTOFMEM,"MemCache alloc memory failed.");
    }
    memset(begin_,0,size_);
    end_ = begin_;
}


MemCache::~MemCache()
{
    free(begin_);

    if (pGrowCache_)
    {
        delete pGrowCache_;
        pGrowCache_ = NULL;
    }
}

uint8_t* MemCache::getMemByLogSize(size_t chunksize)
{
    assert(end_ >= begin_);

    if ((chunksize < MINPOW) || (chunksize > MAXPOW))
        return NULL;

    size_t size = POW_TABLE[chunksize];

    if ((end_ - begin_) + size > size_)
    {
        if (pGrowCache_)
            return pGrowCache_->getMemByLogSize(chunksize);
        return NULL;
    }
    uint8_t* curr = end_;

    end_ += size;

    return curr;
}

uint8_t* MemCache::getMemByRealSize(size_t size)
{
    assert(end_ >= begin_);

    if ((end_ - begin_) + size > size_)
    {
        if (pGrowCache_)
            return pGrowCache_->getMemByRealSize(size);
        return NULL;
    }
    uint8_t* curr = end_;

    end_ += size;

    return curr;
}

void MemCache::flushMem()
{
    assert(end_ >= begin_);

    end_ = begin_;

    if (pGrowCache_)
    {
        delete pGrowCache_;
        pGrowCache_ = NULL;
    }
}

const uint8_t* MemCache::getBegin()
{
    return begin_;
}

const uint8_t* MemCache::getEnd()
{
    return end_;
}

MemCache* MemCache::grow(size_t growSize)
{
    if (pGrowCache_)
        return pGrowCache_->grow(growSize);

    pGrowCache_ = new MemCache(growSize);
    return pGrowCache_;
}

