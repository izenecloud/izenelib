#include <ir/Zambezi/SegmentPool.hpp>
#include <ir/Zambezi/pfordelta/opt_p4.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <algorithm>
#include <cstring>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

SegmentPool::SegmentPool(
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

SegmentPool::~SegmentPool()
{
    for (size_t i = 0; i <= segment_; ++i)
    {
        free(pool_[i]);
    }
}

void SegmentPool::save(std::ostream& ostr) const
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

void SegmentPool::load(std::istream& istr)
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

bool SegmentPool::isTermFrequencyPresent() const
{
    uint32_t reqspace = pool_[0][0];
    uint32_t csize = pool_[0][4];
    if (csize + 5 == reqspace)
    {
        return false;
    }
    return true;
}

bool SegmentPool::isPositional() const
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

size_t SegmentPool::compressAndAddNonPositional(
        uint32_t* docid_list,
        uint32_t len, size_t tailPointer)
{
    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len - 1);
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    uint32_t csize = OPT4(docid_list, len, &block[0], true);

    uint32_t filterSize = bloomEnabled_ ? BloomFilter::computeLength(len, bitsPerElement_) : 0;
    uint32_t reqspace = csize + filterSize + 8;
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
    pool_[segment_][offset_ + 4] = csize + 7;
    pool_[segment_][offset_ + 5] = len;

    pool_[segment_][offset_ + 6] = csize;
    memcpy(&pool_[segment_][offset_ + 7], &block[0], csize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        pool_[segment_][offset_ + csize + 7] = filterSize;
        uint32_t* filter = &pool_[segment_][offset_ + csize + 8];
        for (uint32_t i = 0; i < len; ++i)
        {
            BloomFilter::insert(filter, filterSize, nbHash_, docid_list[i]);
        }
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

    offset_ += reqspace;

    return ENCODE_POINTER(segment_, offset_);
}

size_t SegmentPool::compressAndAddTfOnly(
        uint32_t* docid_list, uint32_t* tf_list,
        uint32_t len, size_t tailPointer)
{
    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len - 1);
        std::reverse(tf_list, tf_list + len - 1);
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> tfblock(BLOCK_SIZE * 2);
    uint32_t csize = OPT4(docid_list, len, &block[0], true);
    uint32_t tfcsize = OPT4(tf_list, len, &tfblock[0], false);

    uint32_t filterSize = bloomEnabled_ ? BloomFilter::computeLength(len, bitsPerElement_) : 0;
    uint32_t reqspace = csize + tfcsize + filterSize + 9;
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
    pool_[segment_][offset_ + 4] = csize + tfcsize + 8;
    pool_[segment_][offset_ + 5] = len;

    pool_[segment_][offset_ + 6] = csize;
    memcpy(&pool_[segment_][offset_ + 7], &block[0], csize * sizeof(uint32_t));

    pool_[segment_][offset_ + csize + 7] = tfcsize;
    memcpy(&pool_[segment_][offset_ + csize + 8], &tfblock[0], tfcsize * sizeof(uint32_t));

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

size_t SegmentPool::compressAndAddPositional(
        uint32_t* docid_list, uint32_t* tf_list, uint32_t* position_list,
        uint32_t len, uint32_t plen, size_t tailPointer)
{
    if (reverse_)
    {
        std::reverse(docid_list, docid_list + len - 1);
        std::reverse(tf_list, tf_list + len - 1);

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

    uint32_t pblocksize = 3 * (plen / BLOCK_SIZE + 1) * BLOCK_SIZE;
    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> tfblock(BLOCK_SIZE * 2);
    std::vector<uint32_t> pblock(pblocksize);
    uint32_t csize = OPT4(docid_list, len, &block[0], true);
    uint32_t tfcsize = OPT4(tf_list, len, &tfblock[0], false);

    uint32_t pcsize = 0;
    uint32_t nb = plen / BLOCK_SIZE;
    uint32_t res = plen % BLOCK_SIZE;

    for (uint32_t i = 0; i < nb; ++i)
    {
        uint32_t tempPcsize = OPT4(&position_list[i * BLOCK_SIZE], BLOCK_SIZE, &pblock[pcsize + 1], false);
        pblock[pcsize] = tempPcsize;
        pcsize += tempPcsize + 1;
    }

    if (res > 0)
    {
        uint32_t tempPcsize = OPT4(&position_list[nb * BLOCK_SIZE], res, &pblock[pcsize + 1], false);
        pblock[pcsize] = tempPcsize;
        pcsize += tempPcsize + 1;
    }

    uint32_t filterSize = bloomEnabled_ ? BloomFilter::computeLength(len, bitsPerElement_) : 0;
    uint32_t reqspace = csize + tfcsize + pcsize + filterSize + 11;
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
        uint32_t* filter = &pool_[segment_][offset_ + csize + tfcsize + pcsize + 11];
        for (uint32_t i = 0; i < len; ++i)
        {
            BloomFilter::insert(filter, filterSize, nbHash_, docid_list[i]);
        }
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

    offset_ += reqspace;

    return ENCODE_POINTER(segment_, offset_);
}

size_t SegmentPool::nextPointer(size_t pointer) const
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

uint32_t SegmentPool::decompressDocidBlock(uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t aux[BLOCK_SIZE*4];
    const uint32_t* block = &pool_[pSegment][pOffset + 7];
    detailed_p4_decode(outBlock, block, aux, true, reverse_);

    return pool_[pSegment][pOffset + 5];
}

uint32_t SegmentPool::decompressTfBlock(uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t aux[BLOCK_SIZE*4];
    uint32_t csize = pool_[pSegment][pOffset + 6];
    const uint32_t* block = &pool_[pSegment][pOffset + csize + 8];
    detailed_p4_decode(outBlock, block, aux, false, reverse_);

    return pool_[pSegment][pOffset + 5];
}

uint32_t SegmentPool::numberOfPositionBlocks(size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_[pSegment][pOffset + 6];
    uint32_t tfsize = pool_[pSegment][pOffset + 7 + csize];
    return pool_[pSegment][pOffset + csize + tfsize + 9];
}

uint32_t SegmentPool::decompressPositionBlock(uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t aux[BLOCK_SIZE * 4];
    uint32_t csize = pool_[pSegment][pOffset + 6];
    uint32_t tfsize = pool_[pSegment][pOffset + 7 + csize];
    uint32_t nb = pool_[pSegment][pOffset + csize + tfsize + 9];

    uint32_t index = pOffset + csize + tfsize + 10;
    for (uint32_t i = 0; i < nb; ++i)
    {
        uint32_t sb = pool_[pSegment][index];
        detailed_p4_decode(&outBlock[i * BLOCK_SIZE], &pool_[pSegment][index + 1], aux, false, reverse_);
        memset(aux, 0, BLOCK_SIZE * 4 * sizeof(uint32_t));
        index += sb + 1;
    }
    return pool_[pSegment][pOffset + csize + tfsize + 8];
}

void SegmentPool::decompressPositions(uint32_t* tf_list, uint32_t index, size_t pointer, uint32_t* out) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t aux[BLOCK_SIZE*4];
    uint32_t csize = pool_[pSegment][pOffset + 6];
    uint32_t tfsize = pool_[pSegment][pOffset + 7 + csize];
    uint32_t nb = 0;
    for (uint32_t i = 0; i < index; ++i)
    {
        nb += tf_list[i];
    }
    uint32_t lnb = nb + tf_list[index] - 1;
    uint32_t r = nb % BLOCK_SIZE;
    nb = nb / BLOCK_SIZE;
    lnb = lnb / BLOCK_SIZE;

    uint32_t pos = pOffset + csize + tfsize + 10;
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
        const uint32_t* block = &pool_[pSegment][pos + 1];
        uint32_t* temp = (uint32_t*) calloc(BLOCK_SIZE * 2, sizeof(uint32_t));
        detailed_p4_decode(temp, block, aux, false, reverse_);
        memcpy(&out[cindex], &temp[rindex], tocopy * sizeof(uint32_t));
        pos += pool_[pSegment][pos] + 1;
        free(temp);

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

bool SegmentPool::containsDocid(uint32_t docid, size_t& pointer) const
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

void SegmentPool::bwandAnd(
        std::vector<size_t>& headPointers,
        uint32_t hits,
        std::vector<uint32_t>& docid_list) const
{
    docid_list.reserve(hits);

    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);
    while (headPointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = decompressDocidBlock(&docid_block[0], headPointers[0]);

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

void SegmentPool::bwandOr(
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

    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);
    while (headPointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = decompressDocidBlock(&docid_block[0], headPointers[0]);

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

void SegmentPool::wand(
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

    for (uint32_t i = 0; i < len; ++i)
    {
        blockDocid[i].resize(BLOCK_SIZE * 2);
        counts[i] = decompressDocidBlock(&blockDocid[i][0], headPointers[i]);
        if (hasTf)
        {
            blockTf[i].resize(BLOCK_SIZE * 2);
            decompressTfBlock(&blockTf[i][0], headPointers[i]);
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

    while (1)
    {
        float sum = 0;
        uint32_t pTerm = -1;
        uint32_t pTermIdx = -1;
        for (uint32_t i = 0; i < len; ++i)
        {
            sum += UB[mapping[i]];
            if (sum > threshold)
            {
                pTerm = mapping[i];
                pTermIdx = i;
                if (blockDocid[mapping[i]][posting[mapping[i]]] !=
                        blockDocid[mapping[i + 1]][posting[mapping[i + 1]]])
                {
                    break;
                }
            }
        }

        if (sum == 0 || pTerm == (uint32_t)-1)
        {
            break;
        }

        uint32_t pivot = blockDocid[pTerm][posting[pTerm]];

        if (blockDocid[mapping[0]][posting[mapping[0]]] == pivot)
        {
            if (pivot != 0)
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
                        threshold = result_list[0].first;
                        if (!hasTf && len == 1) break;
                    }
                }
            }

            for (uint32_t atermIdx = 0; atermIdx < std::min(pTermIdx + 1, len); ++atermIdx)
            {
                uint32_t aterm = mapping[atermIdx];

                if (posting[aterm] >= counts[aterm] - 1 &&
                        nextPointer(headPointers[aterm]) == UNDEFINED_POINTER)
                {
                    uint32_t k = 0;
                    for (uint32_t i = 0; i < len; ++i)
                    {
                        if (i != atermIdx)
                        {
                            mapping[k++] = mapping[i];
                        }
                    }
                    --len;
                    --atermIdx;
                    continue;
                }

                while (LESS_THAN_EQUAL(blockDocid[aterm][posting[aterm]], pivot, reverse_))
                {
                    ++posting[aterm];
                    if (posting[aterm] > counts[aterm] - 1)
                    {
                        headPointers[aterm] = nextPointer(headPointers[aterm]);
                        if (headPointers[aterm] == UNDEFINED_POINTER)
                        {
                            break;
                        }
                        else
                        {
                            counts[aterm] = decompressDocidBlock(&blockDocid[aterm][0], headPointers[aterm]);
                            if (hasTf)
                            {
                                decompressTfBlock(&blockTf[aterm][0], headPointers[aterm]);
                            }
                            posting[aterm] = 0;
                        }
                    }
                }
            }
        }
        else
        {
            uint32_t aterm = mapping[0];
            for (uint32_t atermIdx = 0; atermIdx < std::min(pTermIdx + 1, len); ++atermIdx)
            {
                if (df[mapping[atermIdx]] <= df[aterm] &&
                        LESS_THAN(blockDocid[mapping[atermIdx]][posting[mapping[atermIdx]]], pivot, reverse_))
                {
                    uint32_t atermTemp = mapping[atermIdx];

                    if (posting[atermTemp] >= counts[atermTemp] - 1 &&
                            nextPointer(headPointers[atermTemp]) == UNDEFINED_POINTER)
                    {
                        uint32_t k = 0;
                        for (uint32_t i = 0; i < len; ++i)
                        {
                            if (i != atermIdx)
                            {
                                mapping[k++] = mapping[i];
                            }
                        }
                        --len;
                        --atermIdx;
                        continue;
                    }
                    aterm = atermTemp;
                }
            }

            while (LESS_THAN(blockDocid[aterm][posting[aterm]], pivot, reverse_))
            {
                if (++posting[aterm] > counts[aterm] - 1)
                {
                    headPointers[aterm] = nextPointer(headPointers[aterm]);
                    if (headPointers[aterm] == UNDEFINED_POINTER)
                    {
                        break;
                    }
                    else
                    {
                        counts[aterm] = decompressDocidBlock(&blockDocid[aterm][0], headPointers[aterm]);
                        if (hasTf)
                        {
                            decompressTfBlock(&blockTf[aterm][0], headPointers[aterm]);
                        }
                        posting[aterm] = 0;
                    }
                }
            }
        }

        for (uint32_t i = 0; i < len - 1; ++i)
        {
            uint32_t least = i;
            for (uint32_t j = i + 1; j < len; ++j)
            {
                if (GREATER_THAN(blockDocid[mapping[least]][posting[mapping[least]]],
                                 blockDocid[mapping[j]][posting[mapping[j]]],
                                 reverse_))
                {
                    least = j;
                }
            }
            std::swap(mapping[i], mapping[least]);
        }
    }

    for (uint32_t i = 0; i < result_list.size(); ++i)
    {
        score_list.push_back(result_list[i].first);
        docid_list.push_back(result_list[i].second);
    }
}

}

NS_IZENELIB_IR_END
