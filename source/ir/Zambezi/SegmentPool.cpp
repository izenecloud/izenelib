#include <ir/Zambezi/SegmentPool.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <util/mem_utils.h>

#include <algorithm>
#include <cstring>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

SegmentPool::SegmentPool(uint32_t maxPoolSize, uint32_t numberOfPools)
    : maxPoolSize_(maxPoolSize)
    , numberOfPools_(numberOfPools)
    , segment_(0)
    , offset_(0)
    , pool_(numberOfPools)
{
}

SegmentPool::~SegmentPool()
{
}

void SegmentPool::save(std::ostream& ostr) const
{
    ostr.write((const char*)&maxPoolSize_, sizeof(maxPoolSize_));
    ostr.write((const char*)&numberOfPools_, sizeof(numberOfPools_));
    ostr.write((const char*)&segment_, sizeof(segment_));
    ostr.write((const char*)&offset_, sizeof(offset_));

    for (size_t i = 0; i < segment_; ++i)
    {
        ostr.write((const char*)&pool_[i][0], sizeof(pool_[0][0]) * maxPoolSize_);
    }
    ostr.write((const char*)&pool_[segment_][0], sizeof(pool_[0][0]) * offset_);
}

void SegmentPool::load(std::istream& istr)
{
    istr.read((char*)&maxPoolSize_, sizeof(maxPoolSize_));
    istr.read((char*)&numberOfPools_, sizeof(numberOfPools_));
    istr.read((char*)&segment_, sizeof(segment_));
    istr.read((char*)&offset_, sizeof(offset_));

    pool_.resize(segment_ + 1);
    for (size_t i = 0; i < segment_; ++i)
    {
        if (!pool_[i])
        {
            pool_[i].reset(cachealign_alloc<uint32_t>(maxPoolSize_, HUGEPAGE_SIZE), cachealign_deleter());
        }
        istr.read((char *)&pool_[i][0], sizeof(pool_[0][0]) * maxPoolSize_);
    }
    if (!pool_[segment_])
    {
        pool_[segment_].reset(cachealign_alloc<uint32_t>(maxPoolSize_, HUGEPAGE_SIZE), cachealign_deleter());
    }
    istr.read((char*)&pool_[segment_][0], sizeof(pool_[0][0]) * offset_);
}

size_t SegmentPool::appendSegment(
        uint32_t* dataSegment,
        uint32_t maxDocId,
        uint32_t len,
        size_t lastPointer,
        size_t nextPointer)
{
    uint32_t reqspace = len + 4;
    if (reqspace > maxPoolSize_ - offset_)
    {
        memset(&pool_[segment_][offset_], 0, (maxPoolSize_ - offset_) * sizeof(pool_[0][0]));
        ++segment_;
        offset_ = 0;
    }

    if (!pool_[segment_])
    {
        pool_[segment_].reset(cachealign_alloc<uint32_t>(maxPoolSize_, HUGEPAGE_SIZE), cachealign_deleter());
    }

    pool_[segment_][offset_] = reqspace;
    pool_[segment_][offset_ + 1] = DECODE_SEGMENT(nextPointer);
    pool_[segment_][offset_ + 2] = DECODE_OFFSET(nextPointer);
    pool_[segment_][offset_ + 3] = maxDocId;

    memcpy(&pool_[segment_][offset_ + 4], &dataSegment[0], len * sizeof(pool_[0][0]));

    if (lastPointer != UNDEFINED_POINTER)
    {
        uint32_t lastSegment = DECODE_SEGMENT(lastPointer);
        uint32_t lastOffset = DECODE_OFFSET(lastPointer);

        pool_[lastSegment][lastOffset + 1] = segment_;
        pool_[lastSegment][lastOffset + 2] = offset_;
    }

    size_t newPointer = ENCODE_POINTER(segment_, offset_);
    offset_ += reqspace;

    return newPointer;
}

size_t SegmentPool::nextPointer(size_t pointer) const
{
    if (pointer == UNDEFINED_POINTER)
        return UNDEFINED_POINTER;

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    if (pool_[pSegment][pOffset + 1] == UNDEFINED_SEGMENT)
        return UNDEFINED_POINTER;

    return ENCODE_POINTER(pool_[pSegment][pOffset + 1], pool_[pSegment][pOffset + 2]);
}

size_t SegmentPool::nextPointer(size_t pointer, uint32_t pivot, uint32_t reverse) const
{
    if (pointer == UNDEFINED_POINTER)
        return UNDEFINED_POINTER;

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    while (LESS_THAN(pool_[pSegment][pOffset + 3], pivot, reverse))
    {
        uint32_t oldSegment = pSegment;
        uint32_t oldOffset = pOffset;
        if ((pSegment = pool_[oldSegment][oldOffset + 1]) == UNDEFINED_SEGMENT)
            return UNDEFINED_POINTER;

        pOffset = pool_[oldSegment][oldOffset + 2];
    }

    return ENCODE_POINTER(pSegment, pOffset);
}

}

NS_IZENELIB_IR_END
