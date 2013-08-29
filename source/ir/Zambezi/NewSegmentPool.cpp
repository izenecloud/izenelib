#include <ir/Zambezi/NewSegmentPool.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <algorithm>
#include <cstring>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

NewSegmentPool::NewSegmentPool(uint32_t numberOfPools, bool reverse)
    : numberOfPools_(numberOfPools)
    , segment_(0)
    , offset_(0)
    , reverse_(reverse)
    , pool_(numberOfPools)
{
}

NewSegmentPool::~NewSegmentPool()
{
    for (size_t i = 0; i <= segment_; ++i)
    {
        free(pool_[i]);
    }
}

void NewSegmentPool::save(std::ostream& ostr) const
{
    ostr.write((const char*)&numberOfPools_, sizeof(numberOfPools_));
    ostr.write((const char*)&segment_, sizeof(segment_));
    ostr.write((const char*)&offset_, sizeof(offset_));
    ostr.write((const char*)&reverse_, sizeof(reverse_));

    for (size_t i = 0; i < segment_; ++i)
    {
        ostr.write((const char*)&pool_[i][0], sizeof(uint32_t) * MAX_POOL_SIZE);
    }
    ostr.write((const char*)&pool_[segment_][0], sizeof(uint32_t) * offset_);
}

void NewSegmentPool::load(std::istream& istr)
{
    istr.read((char*)&numberOfPools_, sizeof(numberOfPools_));
    istr.read((char*)&segment_, sizeof(segment_));
    istr.read((char*)&offset_, sizeof(offset_));
    istr.read((char*)&reverse_, sizeof(reverse_));

    pool_.resize(segment_ + 1);
    for (size_t i = 0; i < segment_; ++i)
    {
        pool_[i] = getAlignedMemory(MAX_POOL_SIZE);
        istr.read((char *)&pool_[i][0], sizeof(uint32_t) * MAX_POOL_SIZE);
    }
    pool_[segment_] = getAlignedMemory(MAX_POOL_SIZE);
    istr.read((char*)&pool_[segment_][0], sizeof(uint32_t) * offset_);
}

size_t NewSegmentPool::compressAndAppend(
        SIMDFastPFor& codec,
        uint32_t* docid_list, uint32_t* score_list,
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

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> sblock(BLOCK_SIZE * 2);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);
    size_t scsize = BLOCK_SIZE * 2;
    codec.encodeArray(score_list, BLOCK_SIZE, &sblock[0], scsize);

    uint32_t reqspace = csize + scsize + 7;
    if (reqspace > MAX_POOL_SIZE - offset_)
    {
        ++segment_;
        offset_ = 0;
    }

    if (!pool_[segment_])
    {
        pool_[segment_] = getAlignedMemory(MAX_POOL_SIZE);
    }

    pool_[segment_][offset_] = reqspace;
    pool_[segment_][offset_ + 1] = UNDEFINED_SEGMENT;
    pool_[segment_][offset_ + 2] = 0;
    pool_[segment_][offset_ + 3] = maxDocId;
    pool_[segment_][offset_ + 4] = len;

    pool_[segment_][offset_ + 5] = csize;
    memcpy(&pool_[segment_][offset_ + 6], &block[0], csize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + 6] = scsize;
    memcpy(&pool_[segment_][offset_ + csize + 7], &sblock[0], scsize * sizeof(uint32_t));

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

    offset_ += reqspace;

    return ENCODE_POINTER(segment_, offset_);
}

size_t NewSegmentPool::nextPointer(size_t pointer) const
{
    if (pointer == UNDEFINED_POINTER)
    {
        return UNDEFINED_POINTER;
    }

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    if (pool_[pSegment][pOffset + 1] == UNDEFINED_SEGMENT)
    {
        return UNDEFINED_POINTER;
    }

    return ENCODE_POINTER(pool_[pSegment][pOffset + 1], pool_[pSegment][pOffset + 2]);
}

uint32_t NewSegmentPool::decompressDocidBlock(
        SIMDFastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_[pSegment][pOffset + 6];
    size_t csize = pool_[pSegment][pOffset + 5];
    size_t nvalue = BLOCK_SIZE * 2;
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

uint32_t NewSegmentPool::decompressScoreBlock(
        SIMDFastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 5];
    const uint32_t* block = &pool_[pSegment][pOffset + csize + 7];
    size_t scsize = pool_[pSegment][pOffset + csize + 6];
    size_t nvalue = BLOCK_SIZE * 2;
    codec.decodeArray(block, scsize, outBlock, nvalue);

    return pool_[pSegment][pOffset + 4];
}

}

NS_IZENELIB_IR_END
