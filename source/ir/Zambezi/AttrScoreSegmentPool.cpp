#include <ir/Zambezi/AttrScoreSegmentPool.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <algorithm>
#include <cstring>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

AttrScoreSegmentPool::AttrScoreSegmentPool(uint32_t maxPoolSize, uint32_t numberOfPools, bool reverse)
    : maxPoolSize_(maxPoolSize)
    , numberOfPools_(numberOfPools)
    , segment_(0)
    , offset_(0)
    , reverse_(reverse)
    , pool_(numberOfPools)
{
}

AttrScoreSegmentPool::~AttrScoreSegmentPool()
{
}

void AttrScoreSegmentPool::save(std::ostream& ostr) const
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

void AttrScoreSegmentPool::load(std::istream& istr)
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

size_t AttrScoreSegmentPool::compressAndAppend(
        FastPFor& codec,
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

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, BLOCK_SIZE - len);
        memset(&score_list[len], 0, BLOCK_SIZE - len);
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

    size_t newPointer = ENCODE_POINTER(segment_, offset_);
    offset_ += reqspace;

    return newPointer;
}

size_t AttrScoreSegmentPool::nextPointer(size_t pointer) const
{
    if (pointer == UNDEFINED_POINTER)
        return UNDEFINED_POINTER;

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    if (pool_[pSegment][pOffset + 1] == UNDEFINED_SEGMENT)
        return UNDEFINED_POINTER;

    return ENCODE_POINTER(pool_[pSegment][pOffset + 1], pool_[pSegment][pOffset + 2]);
}

size_t AttrScoreSegmentPool::nextPointer(size_t pointer, uint32_t pivot) const
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

uint32_t AttrScoreSegmentPool::decompressDocidBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_[pSegment][pOffset + 6];
    size_t csize = pool_[pSegment][pOffset + 5];
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

uint32_t AttrScoreSegmentPool::decompressScoreBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 5];
    const uint32_t* block = &pool_[pSegment][pOffset + csize + 7];
    size_t scsize = pool_[pSegment][pOffset + csize + 6];
    size_t nvalue = BLOCK_SIZE;
    codec.decodeArray(block, scsize, outBlock, nvalue);

    return pool_[pSegment][pOffset + 4];
}

bool AttrScoreSegmentPool::gallopSearch_(
        FastPFor& codec,
        std::vector<uint32_t>& blockDocid,
        std::vector<uint32_t>& blockScore,
        uint32_t& count, uint32_t& index, size_t& pointer,
        uint32_t pivot) const
{
    if (LESS_THAN(blockDocid[count - 1], pivot, reverse_))
    {
        if ((pointer = nextPointer(pointer, pivot)) == UNDEFINED_POINTER)
            return false;

        count = decompressDocidBlock(codec, &blockDocid[0], pointer);
        decompressScoreBlock(codec, &blockScore[0], pointer);
        index = 0;
    }

    if (GREATER_THAN_EQUAL(blockDocid[index], pivot, reverse_))
    {
        return true;
    }
    if (blockDocid[count - 1] == pivot)
    {
        index = count - 1;
        return true;
    }

    int beginIndex = index;
    int hop = 1;
    int tempIndex = beginIndex + 1;
    while ((uint32_t)tempIndex < count && LESS_THAN_EQUAL(blockDocid[tempIndex], pivot, reverse_))
    {
        beginIndex = tempIndex;
        tempIndex += hop;
        hop *= 2;
    }
    if (blockDocid[beginIndex] == pivot)
    {
        index = beginIndex;
        return true;
    }

    int endIndex = count - 1;
    hop = 1;
    tempIndex = endIndex - 1;
    while (tempIndex >= 0 && GREATER_THAN(blockDocid[tempIndex], pivot, reverse_))
    {
        endIndex = tempIndex;
        tempIndex -= hop;
        hop *= 2;
    }
    if (blockDocid[endIndex] == pivot)
    {
        index = endIndex;
        return true;
    }

    // Binary search between begin and end indexes
    while (beginIndex < endIndex)
    {
        uint32_t mid = beginIndex + (endIndex - beginIndex) / 2;

        if (LESS_THAN(pivot, blockDocid[mid], reverse_))
        {
            endIndex = mid;
        }
        else if (GREATER_THAN(pivot, blockDocid[mid], reverse_))
        {
            beginIndex = mid + 1;
        }
        else
        {
            index = mid;
            return true;
        }
    }

    index = endIndex;
    return true;
}

void AttrScoreSegmentPool::intersectPostingsLists_(
        FastPFor& codec,
        size_t pointer0,
        size_t pointer1,
        std::vector<uint32_t>& docid_list,
        std::vector<uint32_t>& score_list) const
{
    std::vector<uint32_t> blockDocid0(BLOCK_SIZE);
    std::vector<uint32_t> blockDocid1(BLOCK_SIZE);
    std::vector<uint32_t> blockScore0(BLOCK_SIZE);
    std::vector<uint32_t> blockScore1(BLOCK_SIZE);

    uint32_t c0 = decompressDocidBlock(codec, &blockDocid0[0], pointer0), i0 = 0;
    uint32_t c1 = decompressDocidBlock(codec, &blockDocid1[0], pointer1), i1 = 0;
    decompressScoreBlock(codec, &blockScore0[0], pointer0);
    decompressScoreBlock(codec, &blockScore1[0], pointer1);

    while (true)
    {
        if (blockDocid0[i0] == blockDocid1[i1])
        {
            docid_list.push_back(blockDocid0[i0]);
            score_list.push_back(blockScore0[i0] + blockScore1[i1]);

            if (++i0 == c0)
            {
                if ((pointer0 = nextPointer(pointer0)) == UNDEFINED_POINTER)
                    break;

                c0 = decompressDocidBlock(codec, &blockDocid0[0], pointer0);
                decompressScoreBlock(codec, &blockScore0[0], pointer0);
                i0 = 0;
            }
            if (++i1 == c1)
            {
                if ((pointer1 = nextPointer(pointer1)) == UNDEFINED_POINTER)
                    break;

                c1 = decompressDocidBlock(codec, &blockDocid1[0], pointer1);
                decompressScoreBlock(codec, &blockScore1[0], pointer1);
                i1 = 0;
            }
        }
        else if (LESS_THAN(blockDocid0[i0], blockDocid1[i1], reverse_))
        {
            if (!gallopSearch_(codec, blockDocid0, blockScore0, c0, ++i0, pointer0, blockDocid1[i1]))
                break;
        }
        else
        {
            if (!gallopSearch_(codec, blockDocid1, blockScore1, c1, ++i1, pointer1, blockDocid0[i0]))
                break;
        }
    }
}

void AttrScoreSegmentPool::intersectPostingsLists_(
        FastPFor& codec,
        size_t pointer0,
        size_t pointer1,
        const boost::function<bool(uint32_t)>& filter,
        std::vector<uint32_t>& docid_list,
        std::vector<uint32_t>& score_list) const
{
    std::vector<uint32_t> blockDocid0(BLOCK_SIZE);
    std::vector<uint32_t> blockDocid1(BLOCK_SIZE);
    std::vector<uint32_t> blockScore0(BLOCK_SIZE);
    std::vector<uint32_t> blockScore1(BLOCK_SIZE);

    uint32_t c0 = decompressDocidBlock(codec, &blockDocid0[0], pointer0), i0 = 0;
    uint32_t c1 = decompressDocidBlock(codec, &blockDocid1[0], pointer1), i1 = 0;
    decompressScoreBlock(codec, &blockScore0[0], pointer0);
    decompressScoreBlock(codec, &blockScore1[0], pointer1);

    while (true)
    {
        if (blockDocid0[i0] == blockDocid1[i1])
        {
            if (filter(blockDocid0[i0]))
            {
                docid_list.push_back(blockDocid0[i0]);
                score_list.push_back(blockScore0[i0] + blockScore1[i1]);
            }

            if (++i0 == c0)
            {
                if ((pointer0 = nextPointer(pointer0)) == UNDEFINED_POINTER)
                    break;

                c0 = decompressDocidBlock(codec, &blockDocid0[0], pointer0);
                decompressScoreBlock(codec, &blockScore0[0], pointer0);
                i0 = 0;
            }
            if (++i1 == c1)
            {
                if ((pointer1 = nextPointer(pointer1)) == UNDEFINED_POINTER)
                    break;

                c1 = decompressDocidBlock(codec, &blockDocid1[0], pointer1);
                decompressScoreBlock(codec, &blockScore1[0], pointer1);
                i1 = 0;
            }
        }
        else if (LESS_THAN(blockDocid0[i0], blockDocid1[i1], reverse_))
        {
            if (!gallopSearch_(codec, blockDocid0, blockScore0, c0, ++i0, pointer0, blockDocid1[i1]))
                break;
        }
        else
        {
            if (!gallopSearch_(codec, blockDocid1, blockScore1, c1, ++i1, pointer1, blockDocid0[i0]))
                break;
        }
    }
}

void AttrScoreSegmentPool::intersectSetPostingsList_(
        FastPFor& codec,
        size_t pointer,
        std::vector<uint32_t>& docid_list,
        std::vector<uint32_t>& score_list) const
{
    std::vector<uint32_t> blockDocid(BLOCK_SIZE);
    std::vector<uint32_t> blockScore(BLOCK_SIZE);
    uint32_t c = decompressDocidBlock(codec, &blockDocid[0], pointer);
    decompressScoreBlock(codec, &blockScore[0], pointer);
    uint32_t iSet = 0, iCurrent = 0, i = 0;

    while (iCurrent < docid_list.size())
    {
        if (blockDocid[i] == docid_list[iCurrent])
        {
            docid_list[iSet] = docid_list[iCurrent];
            score_list[iSet++] = score_list[iCurrent] + blockScore[i];

            if (++iCurrent == docid_list.size())
                break;

            if (++i == c)
            {
                if ((pointer = nextPointer(pointer)) == UNDEFINED_POINTER)
                    break;

                c = decompressDocidBlock(codec, &blockDocid[0], pointer);
                decompressScoreBlock(codec, &blockScore[0], pointer);
                i = 0;
            }
        }
        else if (LESS_THAN(blockDocid[i], docid_list[iCurrent], reverse_))
        {
            if (!gallopSearch_(codec, blockDocid, blockScore, c, ++i, pointer, docid_list[iCurrent]))
                break;
        }
        else
        {
            do
            {
                if (++iCurrent == docid_list.size())
                    break;
            }
            while (LESS_THAN(docid_list[iCurrent], blockDocid[i], reverse_));
        }
    }

    docid_list.resize(iSet);
    score_list.resize(iSet);
}

void AttrScoreSegmentPool::intersectSvS(
        std::vector<size_t>& headPointers,
        uint32_t minDf,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<uint32_t>& score_list) const
{
    FastPFor codec;
    if (headPointers.size() < 2)
    {
        std::vector<uint32_t> block(BLOCK_SIZE);
        std::vector<uint32_t> sblock(BLOCK_SIZE);
        uint32_t length = std::min(minDf, hits);
        docid_list.resize(length);
        score_list.resize(length);
        uint32_t iSet = 0;
        size_t t = headPointers[0];
        while (t != UNDEFINED_POINTER && iSet < length)
        {
            uint32_t c = decompressDocidBlock(codec, &block[0], t);
            decompressScoreBlock(codec, &sblock[0], t);
            uint32_t r = iSet + c <= length ? c : length - iSet;
            memcpy(&docid_list[iSet], &block[0], r * sizeof(uint32_t));
            memcpy(&score_list[iSet], &sblock[0], r * sizeof(uint32_t));
            iSet += r;
            t = nextPointer(t);
        }
        return;
    }

    docid_list.reserve(minDf);
    score_list.reserve(minDf);
    intersectPostingsLists_(codec, headPointers[0], headPointers[1], docid_list, score_list);
    for (uint32_t i = 2; i < headPointers.size(); ++i)
    {
        if (docid_list.empty()) break;
        intersectSetPostingsList_(codec, headPointers[i], docid_list, score_list);
    }

    if (hits < docid_list.size())
    {
        docid_list.resize(hits);
        score_list.resize(hits);
    }
}

void AttrScoreSegmentPool::intersectSvS(
        std::vector<size_t>& headPointers,
        const boost::function<bool(uint32_t)>& filter,
        uint32_t minDf,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<uint32_t>& score_list) const
{
    FastPFor codec;
    if (headPointers.size() < 2)
    {
        std::vector<uint32_t> block(BLOCK_SIZE);
        std::vector<uint32_t> sblock(BLOCK_SIZE);
        uint32_t length = std::min(minDf, hits);
        docid_list.reserve(length);
        score_list.reserve(length);
        size_t t = headPointers[0];
        while (t != UNDEFINED_POINTER && docid_list.size() < length)
        {
            uint32_t c = decompressDocidBlock(codec, &block[0], t);
            decompressScoreBlock(codec, &sblock[0], t);
            for (uint32_t i = 0; i < c; ++i)
            {
                if (filter(block[i]))
                {
                    docid_list.push_back(block[i]);
                    score_list.push_back(sblock[i]);
                    if (docid_list.size() == length) break;
                }
            }
            t = nextPointer(t);
        }
        return;
    }

    docid_list.reserve(minDf);
    score_list.reserve(minDf);
    intersectPostingsLists_(codec, headPointers[0], headPointers[1], filter, docid_list, score_list);
    for (uint32_t i = 2; i < headPointers.size(); ++i)
    {
        if (docid_list.empty()) break;
        intersectSetPostingsList_(codec, headPointers[i], docid_list, score_list);
    }

    if (hits < docid_list.size())
    {
        docid_list.resize(hits);
        score_list.resize(hits);
    }
}

}

NS_IZENELIB_IR_END
