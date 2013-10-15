#include <ir/Zambezi/BoolExpSegmentPool.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <algorithm>
#include <cstring>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

BoolExpSegmentPool::BoolExpSegmentPool(uint32_t maxPoolSize, uint32_t numberOfPools, bool reverse)
    : maxPoolSize_(maxPoolSize)
    , numberOfPools_(numberOfPools)
    , segment_(0)
    , offset_(0)
    , reverse_(reverse)
    , pool_(numberOfPools)
{
}

BoolExpSegmentPool::~BoolExpSegmentPool()
{
}

void BoolExpSegmentPool::save(std::ostream& ostr) const
{
    ostr.write((const char*)&maxPoolSize_, sizeof(maxPoolSize_));
    ostr.write((const char*)&numberOfPools_, sizeof(numberOfPools_));
    ostr.write((const char*)&segment_, sizeof(segment_));
    ostr.write((const char*)&offset_, sizeof(offset_));
    ostr.write((const char*)&reverse_, sizeof(reverse_));

    for (size_t i = 0; i < segment_; ++i)
    {
        ostr.write((const char*)&pool_[i][0], sizeof(uint32_t) * maxPoolSize_);
    }
    ostr.write((const char*)&pool_[segment_][0], sizeof(uint32_t) * offset_);
}

void BoolExpSegmentPool::load(std::istream& istr)
{
    istr.read((char*)&maxPoolSize_, sizeof(maxPoolSize_));
    istr.read((char*)&numberOfPools_, sizeof(numberOfPools_));
    istr.read((char*)&segment_, sizeof(segment_));
    istr.read((char*)&offset_, sizeof(offset_));
    istr.read((char*)&reverse_, sizeof(reverse_));

    pool_.resize(segment_ + 1);
    for (size_t i = 0; i < segment_; ++i)
    {
        if (!pool_[i])
        {
            pool_[i].reset(getAlignedIntArray(maxPoolSize_));
        }
        istr.read((char *)&pool_[i][0], sizeof(uint32_t) * maxPoolSize_);
    }
    if (!pool_[segment_])
    {
        pool_[segment_].reset(getAlignedIntArray(maxPoolSize_));
    }
    istr.read((char*)&pool_[segment_][0], sizeof(uint32_t) * offset_);
}

size_t BoolExpSegmentPool::compressAndAppend(
        FastPFor& codec,
        uint32_t* docid_list,
        uint32_t* score_list,
        uint32_t* belong_list,
        uint32_t len, size_t tailPointer)
{
    uint32_t maxDocId = reverse_ ? docid_list[0] : docid_list[len - 1];

    for (uint32_t i = len - 1; i > 0; --i)
    {
        docid_list[i] -= docid_list[i - 1];
    }

    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len);
        std::reverse(score_list, score_list + len);
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, (BLOCK_SIZE - len)*sizeof(uint32_t));
        memset(&score_list[len], 0, (BLOCK_SIZE - len)*sizeof(uint32_t));
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> sblock(BLOCK_SIZE * 2);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);
    size_t scsize = BLOCK_SIZE * 2;
    codec.encodeArray(score_list, BLOCK_SIZE, &sblock[0], scsize);

    uint32_t reqspace = csize + scsize + 7;
    if (reqspace >= maxPoolSize_ - offset_)
    {
        memset(&pool_[segment_][offset_], 0, (maxPoolSize_ - offset_) * sizeof(uint32_t));
        ++segment_;
        offset_ = 0;
    }

    if (!pool_[segment_])
    {
        pool_[segment_].reset(getAlignedIntArray(maxPoolSize_));
    }

    pool_[segment_][offset_] = reqspace;
    pool_[segment_][offset_ + 1] = UNDEFINED_SEGMENT;
    pool_[segment_][offset_ + 2] = UNDEFINED_OFFSET;
    pool_[segment_][offset_ + 3] = maxDocId;
    pool_[segment_][offset_ + 4] = len;

    memcpy(&pool_[segment_][offset_ + 5], &belong_list[0], 4 * sizeof(uint32_t));

    pool_[segment_][offset_ + 9] = csize;
    memcpy(&pool_[segment_][offset_ + 10], &block[0], csize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + 10] = scsize;
    memcpy(&pool_[segment_][offset_ + csize + 11], &sblock[0], scsize * sizeof(uint32_t));

    if (tailPointer != UNDEFINED_POINTER)
    {
        uint32_t lastSegment = DECODE_SEGMENT(tailPointer);
        uint32_t lastOffset = DECODE_OFFSET(tailPointer);
        if (reverse_)
        {
            pool_[segment_][offset_ + 1] = lastSegment;
            pool_[segment_][offset_ + 2] = lastOffset;
        }
        else
        {
            pool_[lastSegment][lastOffset + 1] = segment_;
            pool_[lastSegment][lastOffset + 2] = offset_;
        }
    }

    size_t newPointer = ENCODE_POINTER(segment_, offset_);
    offset_ += reqspace;

    return newPointer;
}

size_t BoolExpSegmentPool::nextPointer(size_t pointer) const
{
    if (pointer == UNDEFINED_POINTER)
        return UNDEFINED_POINTER;

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    if (pool_[pSegment][pOffset + 1] == UNDEFINED_SEGMENT)
        return UNDEFINED_POINTER;

    return ENCODE_POINTER(pool_[pSegment][pOffset + 1], pool_[pSegment][pOffset + 2]);
}

size_t BoolExpSegmentPool::nextPointer(size_t pointer, uint32_t pivot) const
{
    if (pointer == UNDEFINED_POINTER)
        return UNDEFINED_POINTER;

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    do
    {
        uint32_t oldSegment = pSegment;
        uint32_t oldOffset = pOffset;
        if ((pSegment = pool_[oldSegment][oldOffset + 1]) == UNDEFINED_SEGMENT)
            return UNDEFINED_POINTER;

        pOffset = pool_[oldSegment][oldOffset + 2];
    }
    while (LESS_THAN(pool_[pSegment][pOffset + 3], pivot, reverse_));

    return ENCODE_POINTER(pSegment, pOffset);
}

uint32_t BoolExpSegmentPool::decompressDocidBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_[pSegment][pOffset + 10];
    size_t csize = pool_[pSegment][pOffset + 9];
    size_t nvalue = BLOCK_SIZE;
    codec.decodeArray(block, csize, outBlock, nvalue);

    uint32_t len = pool_[pSegment][pOffset + 4];
    if (reverse_)
    {
        for (uint32_t i = len - 1; i > 0; --i)
        {
            outBlock[i - 1] += outBlock[i];
        }
    }
    else
    {
        for (uint32_t i = 1; i < len; ++i)
        {
            outBlock[i] += outBlock[i - 1];
        }
    }

    return len;
}

uint32_t BoolExpSegmentPool::decompressScoreBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 9];
    const uint32_t* block = &pool_[pSegment][pOffset + csize + 11];
    size_t scsize = pool_[pSegment][pOffset + csize + 10];
    size_t nvalue = BLOCK_SIZE;
    codec.decodeArray(block, scsize, outBlock, nvalue);

    return pool_[pSegment][pOffset + 4];
}

}

NS_IZENELIB_IR_END
