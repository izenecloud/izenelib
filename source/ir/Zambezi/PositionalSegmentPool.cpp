#include <ir/Zambezi/PositionalSegmentPool.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <algorithm>
#include <cstring>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

PositionalSegmentPool::PositionalSegmentPool(
        uint32_t maxPoolSize, uint32_t numberOfPools,
        bool reverse, bool bloomEnabled,
        bool nbHash, bool bitsPerElement)
    : maxPoolSize_(maxPoolSize)
    , numberOfPools_(numberOfPools)
    , segment_(0)
    , offset_(0)
    , reverse_(reverse)
    , bloomEnabled_(bloomEnabled)
    , nbHash_(nbHash)
    , bitsPerElement_(bitsPerElement)
    , pool_(numberOfPools)
{
}

PositionalSegmentPool::~PositionalSegmentPool()
{
}

void PositionalSegmentPool::save(std::ostream& ostr) const
{
    ostr.write((const char*)&maxPoolSize_, sizeof(maxPoolSize_));
    ostr.write((const char*)&numberOfPools_, sizeof(numberOfPools_));
    ostr.write((const char*)&segment_, sizeof(segment_));
    ostr.write((const char*)&offset_, sizeof(offset_));
    ostr.write((const char*)&reverse_, sizeof(reverse_));
    ostr.write((const char*)&bloomEnabled_, sizeof(bloomEnabled_));
    ostr.write((const char*)&nbHash_, sizeof(nbHash_));
    ostr.write((const char*)&bitsPerElement_, sizeof(bitsPerElement_));

    for (size_t i = 0; i < segment_; ++i)
    {
        ostr.write((const char*)&pool_[i][0], sizeof(uint32_t) * maxPoolSize_);
    }
    ostr.write((const char*)&pool_[segment_][0], sizeof(uint32_t) * offset_);
}

void PositionalSegmentPool::load(std::istream& istr)
{
    istr.read((char*)&maxPoolSize_, sizeof(maxPoolSize_));
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

bool PositionalSegmentPool::isTermFrequencyPresent() const
{
    uint32_t reqspace = pool_[0][0];
    uint32_t csize = pool_[0][4];
    if (csize + 5 == reqspace)
    {
        return false;
    }
    return true;
}

bool PositionalSegmentPool::isPositional() const
{
    uint32_t reqspace = pool_[0][0];
    uint32_t csize = pool_[0][4];
    if (csize + 5 == reqspace)
    {
        return false;
    }
    uint32_t tfcsize = pool_[0][csize + 5];
    if (csize + tfcsize + 6 == reqspace)
    {
        return false;
    }
    return true;
}

size_t PositionalSegmentPool::compressAndAddNonPositional(
        FastPFor& codec,
        uint32_t* docid_list,
        uint32_t len, size_t tailPointer)
{
    uint32_t maxDocId = reverse_ ? docid_list[0] : docid_list[len - 1];

    uint32_t filterSize = 0;
    std::vector<uint32_t> filter;
    if (bloomEnabled_)
    {
        filterSize = BloomFilter::computeLength(len, bitsPerElement_);
        filter.resize(filterSize);
        for (uint32_t i = 0; i < len; ++i)
        {
            BloomFilter::insert(&filter[0], filterSize, nbHash_, docid_list[i]);
        }
    }

    for (uint32_t i = len - 1; i > 0; --i)
    {
        docid_list[i] -= docid_list[i - 1];
    }

    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len);
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, BLOCK_SIZE - len);
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);

    uint32_t reqspace = csize + filterSize + 8;
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
    pool_[segment_][offset_ + 1] = UNDEFINED_SEGMENT; //save next segment
    pool_[segment_][offset_ + 2] = UNDEFINED_OFFSET; //save next offset;
    pool_[segment_][offset_ + 3] = maxDocId; // max when forward, min when reverse
    pool_[segment_][offset_ + 4] = csize + 7;
    pool_[segment_][offset_ + 5] = len;
    pool_[segment_][offset_ + 6] = csize;
    memcpy(&pool_[segment_][offset_ + 7], &block[0], csize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        pool_[segment_][offset_ + csize + 7] = filterSize;
        memcpy(&pool_[segment_][offset_ + csize + 8], &filter[0], filterSize * sizeof(uint32_t));
    }

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

size_t PositionalSegmentPool::compressAndAddTfOnly(
        FastPFor& codec,
        uint32_t* docid_list, uint32_t* tf_list,
        uint32_t len, size_t tailPointer)
{
    uint32_t maxDocId = reverse_ ? docid_list[0] : docid_list[len - 1];

    uint32_t filterSize = 0;
    std::vector<uint32_t> filter;
    if (bloomEnabled_)
    {
        filterSize = BloomFilter::computeLength(len, bitsPerElement_);
        filter.resize(filterSize);
        for (uint32_t i = 0; i < len; ++i)
        {
            BloomFilter::insert(&filter[0], filterSize, nbHash_, docid_list[i]);
        }
    }

    for (uint32_t i = len - 1; i > 0; --i)
    {
        docid_list[i] -= docid_list[i - 1];
    }

    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len);
        std::reverse(tf_list, tf_list + len);
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, BLOCK_SIZE - len);
        memset(&tf_list[len], 0, BLOCK_SIZE - len);
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> tfblock(BLOCK_SIZE * 2);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);
    size_t tfcsize = BLOCK_SIZE * 2;
    codec.encodeArray(tf_list, BLOCK_SIZE, &tfblock[0], tfcsize);

    uint32_t reqspace = csize + tfcsize + filterSize + 9;
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
    pool_[segment_][offset_ + 4] = csize + tfcsize + 8;
    pool_[segment_][offset_ + 5] = len;

    pool_[segment_][offset_ + 6] = csize;
    memcpy(&pool_[segment_][offset_ + 7], &block[0], csize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + 7] = tfcsize;
    memcpy(&pool_[segment_][offset_ + csize + 8], &tfblock[0], tfcsize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        pool_[segment_][offset_ + csize + tfcsize + 8] = filterSize;
        memcpy(&pool_[segment_][offset_ + csize + tfcsize + 9], &filter[0], filterSize * sizeof(uint32_t));
    }

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

size_t PositionalSegmentPool::compressAndAddPositional(
        FastPFor& codec,
        uint32_t* docid_list, uint32_t* tf_list, uint32_t* position_list,
        uint32_t len, uint32_t plen, size_t tailPointer)
{
    uint32_t maxDocId = reverse_ ? docid_list[0] : docid_list[len - 1];

    uint32_t filterSize = 0;
    std::vector<uint32_t> filter;
    if (bloomEnabled_)
    {
        filterSize = BloomFilter::computeLength(len, bitsPerElement_);
        filter.resize(filterSize);
        for (uint32_t i = 0; i < len; ++i)
        {
            BloomFilter::insert(&filter[0], filterSize, nbHash_, docid_list[i]);
        }
    }

    for (uint32_t i = len - 1; i > 0; --i)
    {
        docid_list[i] -= docid_list[i - 1];
    }

    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len);
        std::reverse(tf_list, tf_list + len);

        std::vector<uint32_t> rpositions(plen);
        uint32_t curPos = plen, newPos = 0;
        for (uint32_t i = 0; i < len; ++i)
        {
            curPos -= tf_list[i];
            memcpy(&rpositions[newPos], &position_list[curPos], tf_list[i] * sizeof(uint32_t));
            newPos += tf_list[i];
        }
        memcpy(position_list, &rpositions[0], plen * sizeof(uint32_t));
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, BLOCK_SIZE - len);
        memset(&tf_list[len], 0, BLOCK_SIZE - len);
    }

    uint32_t pblocksize = 3 * (plen / BLOCK_SIZE + 1) * BLOCK_SIZE;
    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> tfblock(BLOCK_SIZE * 2);
    std::vector<uint32_t> pblock(pblocksize);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);
    size_t tfcsize = BLOCK_SIZE * 2;
    codec.encodeArray(tf_list, BLOCK_SIZE, &tfblock[0], tfcsize);

    uint32_t pcsize = 0;
    uint32_t nb = plen / BLOCK_SIZE;
    uint32_t res = plen % BLOCK_SIZE;

    for (uint32_t i = 0; i < nb; ++i)
    {
        size_t tempPcsize = BLOCK_SIZE * 2;
        codec.encodeArray(&position_list[i * BLOCK_SIZE], BLOCK_SIZE, &pblock[pcsize + 1], tempPcsize);
        pblock[pcsize] = tempPcsize;
        pcsize += tempPcsize + 1;
    }

    if (res > 0)
    {
        memset(&position_list[plen], 0, BLOCK_SIZE - res);
        size_t tempPcsize = BLOCK_SIZE * 2;
        codec.encodeArray(&position_list[nb * BLOCK_SIZE], BLOCK_SIZE, &pblock[pcsize + 1], tempPcsize);
        pblock[pcsize] = tempPcsize;
        pcsize += tempPcsize + 1;
    }

    uint32_t reqspace = csize + tfcsize + pcsize + filterSize + 11;
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
    pool_[segment_][offset_ + 4] = csize + tfcsize + pcsize + 10;
    pool_[segment_][offset_ + 5] = len;

    pool_[segment_][offset_ + 6] = csize;
    memcpy(&pool_[segment_][offset_ + 7], &block[0], csize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + 7] = tfcsize;
    memcpy(&pool_[segment_][offset_ + csize + 8], &tfblock[0], tfcsize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + tfcsize + 8] = plen;
    pool_[segment_][offset_ + csize + tfcsize + 9] = nb + (res ? 1 : 0);
    memcpy(&pool_[segment_][offset_ + csize + tfcsize + 10], &pblock[0], pcsize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        pool_[segment_][offset_ + csize + tfcsize + pcsize + 10] = filterSize;
        memcpy(&pool_[segment_][offset_ + csize + tfcsize + pcsize + 11], &filter[0], filterSize * sizeof(uint32_t));
    }

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

size_t PositionalSegmentPool::nextPointer(size_t pointer) const
{
    if (pointer == UNDEFINED_POINTER)
        return UNDEFINED_POINTER;

    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    if (pool_[pSegment][pOffset + 1] == UNDEFINED_SEGMENT)
        return UNDEFINED_POINTER;

    return ENCODE_POINTER(pool_[pSegment][pOffset + 1], pool_[pSegment][pOffset + 2]);
}

size_t PositionalSegmentPool::nextPointer(size_t pointer, uint32_t pivot) const
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


uint32_t PositionalSegmentPool::decompressDocidBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_[pSegment][pOffset + 7];
    size_t csize = pool_[pSegment][pOffset + 6];
    size_t nvalue = BLOCK_SIZE;
    memset(outBlock, 0, BLOCK_SIZE * sizeof(uint32_t));
    codec.decodeArray(block, csize, outBlock, nvalue);

    uint32_t len = pool_[pSegment][pOffset + 5];

    if (len == 0)
        return 0;

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

uint32_t PositionalSegmentPool::decompressTfBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 6];
    const uint32_t* block = &pool_[pSegment][pOffset + csize + 8];
    size_t tfcsize = pool_[pSegment][pOffset + csize + 7];
    size_t nvalue = BLOCK_SIZE;
    memset(outBlock, 0, BLOCK_SIZE * sizeof(uint32_t));
    codec.decodeArray(block, tfcsize, outBlock, nvalue);

    return pool_[pSegment][pOffset + 5];
}

uint32_t PositionalSegmentPool::numberOfPositionBlocks(size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_[pSegment][pOffset + csize + 7];
    return pool_[pSegment][pOffset + csize + tfcsize + 9];
}

uint32_t PositionalSegmentPool::decompressPositionBlock(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_[pSegment][pOffset + csize + 7];
    uint32_t nb = pool_[pSegment][pOffset + csize + tfcsize + 9];

    uint32_t index = pOffset + csize + tfcsize + 10;
    for (uint32_t i = 0; i < nb; ++i)
    {
        uint32_t sb = pool_[pSegment][index];
        size_t nvalue = BLOCK_SIZE;
        codec.decodeArray(&pool_[pSegment][index + 1], sb, &outBlock[i * BLOCK_SIZE], nvalue);
        index += sb + 1;
    }
    return pool_[pSegment][pOffset + csize + tfcsize + 8];
}

void PositionalSegmentPool::decompressPositions(
        FastPFor& codec,
        uint32_t* tf_list, uint32_t index, size_t pointer, uint32_t* out) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_[pSegment][pOffset + 7 + csize];
    uint32_t nb = 0;
    for (uint32_t i = 0; i < index; ++i)
    {
        nb += tf_list[i];
    }
    uint32_t lnb = nb + tf_list[index] - 1;
    uint32_t r = nb % BLOCK_SIZE;
    nb = nb / BLOCK_SIZE;
    lnb = lnb / BLOCK_SIZE;

    uint32_t pos = pOffset + csize + tfcsize + 10;
    for (uint32_t i = 0; i < nb; ++i)
    {
        pos += pool_[pSegment][pos] + 1;
    }
    uint32_t cindex = 0, left = tf_list[index], tocopy = tf_list[index], rindex = r;
    for (uint32_t i = nb; i <= lnb; ++i)
    {
        if (rindex + tocopy > BLOCK_SIZE)
        {
            tocopy = BLOCK_SIZE - rindex;
        }
        uint32_t sb = pool_[pSegment][pos];
        std::vector<uint32_t> temp(BLOCK_SIZE);
        size_t nvalue = BLOCK_SIZE;
        codec.decodeArray(&pool_[pSegment][pos + 1], sb, &temp[0], nvalue);
        memcpy(&out[cindex], &temp[rindex], tocopy * sizeof(uint32_t));
        pos += sb + 1;

        cindex += tocopy;
        left -= tocopy;
        tocopy = left;
        rindex = 0;
    }
    for (uint32_t i = 1; i < tf_list[index]; ++i)
    {
        out[i] += out[i - 1];
    }
}

bool PositionalSegmentPool::containsDocid(uint32_t docid, size_t& pointer) const
{
    if (pointer == UNDEFINED_POINTER)
    {
        return false;
    }
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    while (LESS_THAN(pool_[pSegment][pOffset + 3], docid, reverse_))
    {
        uint32_t oldSegment = pSegment;
        uint32_t oldOffset = pOffset;
        pSegment = pool_[oldSegment][oldOffset + 1];
        if (pSegment == UNDEFINED_SEGMENT)
        {
            pointer = UNDEFINED_POINTER;
            return false;
        }
        pOffset = pool_[oldSegment][oldOffset + 2];
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

void PositionalSegmentPool::bwandAnd(
        std::vector<size_t>& headPointers,
        uint32_t hits,
        std::vector<uint32_t>& docid_list) const
{
    docid_list.reserve(hits);
    FastPFor codec;
    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);

    while (headPointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = decompressDocidBlock(codec, &docid_block[0], headPointers[0]);
        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t pivot = docid_block[i];
            bool found = true;
            for (uint32_t j = 1; j < headPointers.size(); ++j)
            {
                if (headPointers[j] == UNDEFINED_POINTER)
                    return;

                if (!containsDocid(pivot, headPointers[j]))
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                docid_list.push_back(pivot);
                if (docid_list.size() >= hits)
                    return;
            }
        }
        headPointers[0] = nextPointer(headPointers[0]);
    }
}

void PositionalSegmentPool::bwandOr(
        std::vector<size_t>& headPointers,
        const std::vector<float>& UB,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    docid_list.reserve(hits);
    score_list.reserve(hits);

    std::vector<std::pair<float, uint32_t> > result_list;
    result_list.reserve(hits);
    std::greater<std::pair<float, uint32_t> > comparator;

    float threshold = .0f;
    float sumOfUB = .0f;
    for (uint32_t i = 0; i < UB.size(); ++i)
    {
        sumOfUB += UB[i];
    }

    FastPFor codec;
    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);
    while (headPointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = decompressDocidBlock(codec, &docid_block[0], headPointers[0]);

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t pivot = docid_block[i];
            float score = UB[0];
            for (uint32_t j = 1; j < headPointers.size(); ++j)
            {
                if (containsDocid(pivot, headPointers[j]))
                {
                    score += UB[j];
                }
            }

            if (result_list.size() < hits)
            {
                result_list.push_back(std::make_pair(score, pivot));
                std::push_heap(result_list.begin(), result_list.end(), comparator);
                if (result_list.size() == hits && (threshold = result_list[0].first) == sumOfUB)
                    break;
            }
            else if (score > threshold)
            {
                std::pop_heap(result_list.begin(), result_list.end(), comparator);
                result_list.back() = std::make_pair(score, pivot);
                std::push_heap(result_list.begin(), result_list.end(), comparator);
                if ((threshold = result_list[0].first) == sumOfUB)
                    break;
            }
        }

        if (result_list.size() == hits && threshold == sumOfUB)
            break;

        headPointers[0] = nextPointer(headPointers[0]);
    }

    for (uint32_t i = 0; i < result_list.size(); ++i)
    {
        score_list.push_back(result_list[i].first);
        docid_list.push_back(result_list[i].second);
    }
}

bool PositionalSegmentPool::gallopSearch_(
        FastPFor& codec,
        std::vector<uint32_t>& blockDocid,
        uint32_t& count, uint32_t& index, size_t& pointer,
        uint32_t pivot) const
{
    if (LESS_THAN(blockDocid[count - 1], pivot, reverse_))
    {
        if ((pointer = nextPointer(pointer, pivot)) == UNDEFINED_POINTER)
            return false;

        count = decompressDocidBlock(codec, &blockDocid[0], pointer);
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

void PositionalSegmentPool::wand(
        std::vector<size_t>& headPointers,
        const std::vector<uint32_t>& df,
        const std::vector<float>& UB,
        const std::vector<uint32_t>& docLen,
        uint32_t totalDocs,
        float avgDocLen,
        uint32_t hits,
        bool hasTf,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    uint32_t len = headPointers.size();
    std::vector<std::vector<uint32_t> > blockDocid(len);
    std::vector<std::vector<uint32_t> > blockTf(len);
    std::vector<uint32_t> counts(len);
    std::vector<uint32_t> posting(len);
    std::vector<uint32_t> mapping(len);
    float threshold = .0f;

    FastPFor codec;
    for (uint32_t i = 0; i < len; ++i)
    {
        blockDocid[i].resize(BLOCK_SIZE);
        counts[i] = decompressDocidBlock(codec, &blockDocid[i][0], headPointers[i]);
        if (hasTf)
        {
            blockTf[i].resize(BLOCK_SIZE);
            decompressTfBlock(codec, &blockTf[i][0], headPointers[i]);
        }
        mapping[i] = i;
        if (UB[i] <= threshold)
        {
            threshold = UB[i] - 1;
        }
    }

    for (uint32_t i = 0; i < len - 1; ++i)
    {
        uint32_t least = i;
        for (uint32_t j = i + 1; j < len; ++j)
        {
            if (GREATER_THAN(blockDocid[mapping[least]][0],
                             blockDocid[mapping[j]][0],
                             reverse_))
            {
                least = j;
            }
        }
        std::swap(mapping[i], mapping[least]);
    }

    std::vector<std::pair<float, uint32_t> > result_list;
    result_list.reserve(hits);
    std::greater<std::pair<float, uint32_t> > comparator;

    while (true)
    {
        float sum = 0;
        uint32_t pTermIdx = -1;
        for (uint32_t i = 0; i < len; ++i)
        {
            if ((sum += UB[mapping[i]]) > threshold)
            {
                pTermIdx = i;
                if (i == len - 1 || blockDocid[mapping[i]][posting[mapping[i]]] !=
                        blockDocid[mapping[i + 1]][posting[mapping[i + 1]]])
                {
                    break;
                }
            }
        }

        if (sum == 0 || pTermIdx == (uint32_t)-1)
        {
            break;
        }

        uint32_t pTerm = mapping[pTermIdx];
        uint32_t pivot = blockDocid[pTerm][posting[pTerm]];

        if (blockDocid[mapping[0]][posting[mapping[0]]] == pivot)
        {
            float score = 0;
            if (hasTf)
            {
                for (uint32_t i = 0; i <= pTermIdx; ++i)
                {
                    score += default_bm25(blockTf[mapping[i]][posting[mapping[i]]],
                            df[mapping[i]], totalDocs, docLen[pivot], avgDocLen);
                }
            }
            else
            {
                score = sum;
            }

            if (score > threshold)
            {
                if (result_list.size() < hits)
                {
                    result_list.push_back(std::make_pair(score, pivot));
                    std::push_heap(result_list.begin(), result_list.end(), comparator);
                    if (result_list.size() == hits)
                    {
                        if (!hasTf && len == 1) break;
                        threshold = result_list[0].first;
                    }
                }
                else if (score > result_list[0].first)
                {
                    std::pop_heap(result_list.begin(), result_list.end(), comparator);
                    result_list.back() = std::make_pair(score, pivot);
                    std::push_heap(result_list.begin(), result_list.end(), comparator);
                    if (!hasTf && len == 1) break;
                    threshold = result_list[0].first;
                }
            }

            for (uint32_t atermIdx = 0; atermIdx <= pTermIdx; ++atermIdx)
            {
                uint32_t aterm = mapping[atermIdx];

                if (posting[aterm] == counts[aterm] - 1)
                {
                    if ((headPointers[aterm] = nextPointer(headPointers[aterm])) == UNDEFINED_POINTER)
                    {
                        mapping.erase(mapping.begin() + atermIdx);
                        --len;
                        --atermIdx;
                        continue;
                    }
                    else
                    {
                        counts[aterm] = decompressDocidBlock(codec, &blockDocid[aterm][0], headPointers[aterm]);
                        if (hasTf)
                        {
                            decompressTfBlock(codec, &blockTf[aterm][0], headPointers[aterm]);
                        }
                        posting[aterm] = 0;
                    }
                }
            }
        }
        else
        {
            for (uint32_t atermIdx = 0; atermIdx <= pTermIdx; ++atermIdx)
            {
                if (blockDocid[mapping[atermIdx]][posting[mapping[atermIdx]]] == pivot)
                    break;

                uint32_t aterm = mapping[atermIdx];
                size_t tmpHead = headPointers[aterm];
                if (!gallopSearch_(codec, blockDocid[aterm], counts[aterm], posting[aterm], headPointers[aterm], pivot))
                {
                    mapping.erase(mapping.begin() + atermIdx);
                    --len;
                    --atermIdx;
                }
                else if (hasTf && tmpHead != headPointers[aterm])
                {
                    decompressTfBlock(codec, &blockTf[aterm][0], headPointers[aterm]);
                }
            }
        }

        for (uint32_t i = 0; i <= pTermIdx; ++i)
        {
            bool unchanged = true;
            for (uint32_t j = i + 1; j < len; ++j)
            {
                if (GREATER_THAN(blockDocid[mapping[j - 1]][posting[mapping[j - 1]]],
                                 blockDocid[mapping[j]][posting[mapping[j]]],
                                 reverse_))
                {
                    std::swap(mapping[j - 1], mapping[j]);
                    unchanged = false;
                }
            }
            if (unchanged) break;
        }
    }

    std::sort_heap(result_list.begin(), result_list.end(), comparator);
    for (uint32_t i = 0; i < result_list.size(); ++i)
    {
        score_list.push_back(result_list[i].first);
        docid_list.push_back(result_list[i].second);
    }
}

void PositionalSegmentPool::intersectPostingsLists_(
        FastPFor& codec,
        size_t pointer0, size_t pointer1,
        std::vector<uint32_t>& docid_list) const
{
    std::vector<uint32_t> data0(BLOCK_SIZE);
    std::vector<uint32_t> data1(BLOCK_SIZE);

    uint32_t c0 = decompressDocidBlock(codec, &data0[0], pointer0);
    uint32_t c1 = decompressDocidBlock(codec, &data1[0], pointer1);
    uint32_t i0 = 0, i1 = 0;

    while (true)
    {
        if (data1[i1] == data0[i0])
        {
            docid_list.push_back(data0[i0]);
            if (++i0 == c0)
            {
                if ((pointer0 = nextPointer(pointer0)) == UNDEFINED_POINTER)
                    break;

                c0 = decompressDocidBlock(codec, &data0[0], pointer0);
                i0 = 0;
            }
            if (++i1 == c1)
            {
                if ((pointer1 = nextPointer(pointer1)) == UNDEFINED_POINTER)
                    break;

                c1 = decompressDocidBlock(codec, &data1[0], pointer1);
                i1 = 0;
            }
        }

        if (LESS_THAN(data0[i0], data1[i1], reverse_))
        {
            if (!gallopSearch_(codec, data0, c0, ++i0, pointer0, data1[i1]))
                break;
        }
        else if (LESS_THAN(data1[i1], data0[i0], reverse_))
        {
            if (!gallopSearch_(codec, data1, c1, ++i1, pointer1, data0[i0]))
                break;
        }
    }
}

void PositionalSegmentPool::intersectSetPostingsList_(
        FastPFor& codec,
        size_t pointer,
        std::vector<uint32_t>& docid_list) const
{
    std::vector<uint32_t> blockDocid(BLOCK_SIZE);
    uint32_t c = decompressDocidBlock(codec, &blockDocid[0], pointer);
    uint32_t iSet = 0, iCurrent = 0, i = 0;

    while (iCurrent < docid_list.size())
    {
        if (blockDocid[i] == docid_list[iCurrent])
        {
            docid_list[iSet++] = docid_list[iCurrent++];
            if (iCurrent == docid_list.size())
                break;

            if (++i == c)
            {
                if ((pointer = nextPointer(pointer)) == UNDEFINED_POINTER)
                    break;

                c = decompressDocidBlock(codec, &blockDocid[0], pointer);
                i = 0;
            }
        }
        else if (LESS_THAN(blockDocid[i], docid_list[iCurrent], reverse_))
        {
            if (!gallopSearch_(codec, blockDocid, c, ++i, pointer, docid_list[iCurrent]))
                break;
        }
        else
        {
            while (iCurrent < docid_list.size() && LESS_THAN(docid_list[iCurrent], blockDocid[i], reverse_))
            {
                ++iCurrent;
            }
            if (iCurrent == docid_list.size())
                break;
        }
    }

    docid_list.resize(iSet);
}

void PositionalSegmentPool::intersectSvS(
        std::vector<size_t>& headPointers,
        uint32_t minDf,
        uint32_t hits,
        std::vector<uint32_t>& docid_list) const
{
    FastPFor codec;
    if (headPointers.size() < 2)
    {
        std::vector<uint32_t> block(BLOCK_SIZE);
        uint32_t length = std::min(minDf, hits);
        docid_list.resize(length);
        uint32_t iSet = 0;
        size_t t = headPointers[0];
        while (t != UNDEFINED_POINTER && iSet < length)
        {
            uint32_t c = decompressDocidBlock(codec, &block[0], t);
            uint32_t r = iSet + c <= length ? c : length - iSet;
            memcpy(&docid_list[iSet], &block[0], r * sizeof(uint32_t));
            iSet += r;
            t = nextPointer(t);
        }
        return;
    }

    docid_list.reserve(minDf);
    intersectPostingsLists_(codec, headPointers[0], headPointers[1], docid_list);
    for (uint32_t i = 2; i < headPointers.size(); ++i)
    {
        if (docid_list.empty()) break;
        intersectSetPostingsList_(codec, headPointers[i], docid_list);
    }

    if (hits < docid_list.size())
        docid_list.resize(hits);
}

}

NS_IZENELIB_IR_END
