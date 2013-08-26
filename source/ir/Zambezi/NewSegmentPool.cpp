#include <ir/Zambezi/NewSegmentPool.hpp>
#include <ir/Zambezi/pfordelta/opt_p4.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <algorithm>
#include <cstring>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

NewSegmentPool::NewSegmentPool(
        uint32_t numberOfPools, bool reverse, bool bloomEnabled,
        bool nbHash, bool bitsPerElement)
    : numberOfPools_(numberOfPools)
    , segment_(0)
    , offset_(0)
    , reverse_(reverse)
    , bloomEnabled_(bloomEnabled)
    , nbHash_(nbHash)
    , bitsPerElement_(bitsPerElement)
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
    ostr.write((const char*)&bloomEnabled_, sizeof(bloomEnabled_));
    ostr.write((const char*)&nbHash_, sizeof(nbHash_));
    ostr.write((const char*)&bitsPerElement_, sizeof(bitsPerElement_));

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
    istr.read((char*)&bloomEnabled_, sizeof(bloomEnabled_));
    istr.read((char*)&nbHash_, sizeof(nbHash_));
    istr.read((char*)&bitsPerElement_, sizeof(bitsPerElement_));

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
        uint32_t* docid_list, uint32_t* score_list,
        uint32_t len, size_t tailPointer)
{
    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len);
        std::reverse(score_list, score_list + len);
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> sblock(BLOCK_SIZE * 2);
    uint32_t csize = OPT4(docid_list, len, &block[0], true);
    uint32_t scsize = OPT4(score_list, len, &sblock[0], false);

    uint32_t filterSize = bloomEnabled_ ? BloomFilter::computeLength(len, bitsPerElement_) : 0;
    uint32_t reqspace = csize + scsize + filterSize + 9;
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
    pool_[segment_][offset_ + 3] = reverse_ ? docid_list[0] : docid_list[len - 1];
    pool_[segment_][offset_ + 4] = csize + scsize + 8;
    pool_[segment_][offset_ + 5] = len;

    pool_[segment_][offset_ + 6] = csize;
    memcpy(&pool_[segment_][offset_ + 7], &block[0], csize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + 7] = scsize;
    memcpy(&pool_[segment_][offset_ + csize + 8], &sblock[0], scsize * sizeof(uint32_t));

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

uint32_t NewSegmentPool::decompressDocidBlock(uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t aux[BLOCK_SIZE*4];
    const uint32_t* block = &pool_[pSegment][pOffset + 7];
    detailed_p4_decode(outBlock, block, aux, true, reverse_);

    return pool_[pSegment][pOffset + 5];
}

uint32_t NewSegmentPool::decompressScoreBlock(uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t aux[BLOCK_SIZE*4];
    uint32_t csize = pool_[pSegment][pOffset + 6];
    const uint32_t* block = &pool_[pSegment][pOffset + csize + 8];
    detailed_p4_decode(outBlock, block, aux, false, reverse_);

    return pool_[pSegment][pOffset + 5];
}

bool NewSegmentPool::containsDocid(uint32_t docid, size_t& pointer) const
{
    if (pointer == UNDEFINED_POINTER)
    {
        return false;
    }
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    while (LESS_THAN(pool_[pSegment][pOffset + 3], docid, reverse_))
    {
        pSegment = pool_[pSegment][pOffset + 1];
        pOffset = pool_[pSegment][pOffset + 2];
        if (pSegment == UNDEFINED_SEGMENT)
        {
            pointer = UNDEFINED_POINTER;
            return false;
        }
    }

    if (pool_[pSegment][pOffset + 3] == docid)
    {
        return true;
    }

    uint32_t bloomOffset = pool_[pSegment][pOffset + 4];
    pointer = ENCODE_POINTER(pSegment, pOffset);
    return BloomFilter::contains(
            &pool_[pSegment][pOffset + bloomOffset + 1],
            pool_[pSegment][pOffset + bloomOffset],
            nbHash_, docid);
}

}

NS_IZENELIB_IR_END
