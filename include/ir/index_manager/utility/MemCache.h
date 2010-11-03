/**
* @file        MemCache.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief The memory pool
*/
#ifndef MEMCACHE_H
#define MEMCACHE_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>

#include <cmath>
#include <vector>

#define MINPOW 5
#define MAXPOW 31

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///memory pool, created when IndexBarrelWriter is first used.
class MemCache
{
public:
    MemCache(size_t cachesize);

    ~MemCache();
public:
    uint8_t* getMemByLogSize(size_t chunksize);

    uint8_t* getMemByRealSize(size_t size);

    void flushMem();

    const uint8_t* getBegin();

    const uint8_t* getEnd();

    const size_t getSize() { return size_; }

    bool isEmpty() { return (end_==begin_); }
    /**
    * If the initial size of the memory has been exhausted, this function will apply for more memory
    */
    MemCache* grow(size_t growSize);
    ///Judge whether memory has grown, if true, it means the memory allocated is not so enough, some
    ///strategy should be adopted such as flushing InMemoryPosting to barrels
    bool isGrowed() { return pGrowCache_!=NULL; }

private:
    uint8_t* begin_;

    uint8_t* end_;

    size_t size_;

    MemCache* pGrowCache_;
};

}

NS_IZENELIB_IR_END


#endif
