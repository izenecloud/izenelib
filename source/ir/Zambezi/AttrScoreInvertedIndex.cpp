#include <ir/Zambezi/AttrScoreInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <glog/logging.h>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

AttrScoreInvertedIndex::AttrScoreInvertedIndex(
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        uint32_t vocabSize,
        bool reverse)
    : buffer_(vocabSize)
    , pool_(maxPoolSize, numberOfPools, reverse)
    , dictionary_(vocabSize)
    , pointers_(vocabSize, 0)
{
}

AttrScoreInvertedIndex::~AttrScoreInvertedIndex()
{
}

void AttrScoreInvertedIndex::save(std::ostream& ostr) const
{
    LOG(INFO) << "Save: start....";

    std::streamoff offset = ostr.tellp();
    buffer_.save(ostr);
    LOG(INFO) << "Saved: buffer maps size " << ostr.tellp() - offset;
    offset = ostr.tellp();
    pool_.save(ostr);
    LOG(INFO) << "Saved: segment pools size " << ostr.tellp() - offset;
    offset = ostr.tellp();
    dictionary_.save(ostr);
    LOG(INFO) << "Saved: dictionary size " << ostr.tellp() - offset;
    offset = ostr.tellp();
    pointers_.save(ostr);
    LOG(INFO) << "Saved: head pointers size " << ostr.tellp() - offset;

    LOG(INFO) << "Save: done!";
}

void AttrScoreInvertedIndex::load(std::istream& istr)
{
    LOG(INFO) << "Load: start....";

    std::streamoff offset = istr.tellg();
    buffer_.load(istr);
    LOG(INFO) << "Loaded: buffer maps size " << istr.tellg() - offset;
    offset = istr.tellg();
    pool_.load(istr);
    LOG(INFO) << "Loaded: segment pool size " << istr.tellg() - offset;
    offset = istr.tellg();
    dictionary_.load(istr);
    LOG(INFO) << "Loaded: dictionary size " << istr.tellg() - offset;
    offset = istr.tellg();
    pointers_.load(istr);
    LOG(INFO) << "Loaded: head pointers size " << istr.tellg() - offset;

    LOG(INFO) << "Load: done!";
}

uint32_t AttrScoreInvertedIndex::totalDocNum() const
{
    return pointers_.totalDocs_;
}

void AttrScoreInvertedIndex::insertDoc(
        uint32_t docid,
        const std::vector<std::string>& term_list,
        const std::vector<uint32_t>& score_list)
{
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t id = dictionary_.insertTerm(term_list[i]);

        if (id == INVALID_ID)
        {
            LOG(WARNING) << "failed to insert term as dictionary is full"
                         << ", term: " << term_list[i]
                         << ", dictionary size: " << dictionary_.size();
            continue;
        }

        pointers_.df_.increment(id);
        pointers_.cf_.increment(id);
        std::vector<uint32_t>& docBuffer = buffer_.docid_[id];
        std::vector<uint32_t>& scoreBuffer = buffer_.score_[id];

        if (pointers_.df_.get(id) <= DF_CUTOFF)
        {
            if (docBuffer.capacity() == 0)
            {
                docBuffer.reserve(DF_CUTOFF);
                scoreBuffer.reserve(DF_CUTOFF);
            }
            docBuffer.push_back(docid);
            scoreBuffer.push_back(score_list[i]);
            continue;
        }

        if (docBuffer.capacity() < BLOCK_SIZE)
        {
            docBuffer.reserve(BLOCK_SIZE);
            scoreBuffer.reserve(BLOCK_SIZE);
        }

        docBuffer.push_back(docid);
        scoreBuffer.push_back(score_list[i]);

        if (docBuffer.size() == docBuffer.capacity())
        {
            processTermBuffer_(
                    docBuffer,
                    scoreBuffer,
                    buffer_.tailPointer_[id],
                    pointers_.headPointers_.get(id));

            if (docBuffer.capacity() < MAX_BLOCK_SIZE)
            {
                uint32_t newLen = docBuffer.capacity() * EXPANSION_RATE;
                docBuffer.reserve(newLen);
                scoreBuffer.reserve(newLen);
            }
        }
    }
    ++pointers_.totalDocs_;
}

void AttrScoreInvertedIndex::flush()
{
    uint32_t term = INVALID_ID;
    while ((term = buffer_.nextIndex(term, DF_CUTOFF)) != INVALID_ID)
    {
        processTermBuffer_(
                buffer_.docid_[term],
                buffer_.score_[term],
                buffer_.tailPointer_[term],
                pointers_.headPointers_.get(term));
    }
}

void AttrScoreInvertedIndex::processTermBuffer_(
        std::vector<uint32_t>& docBuffer,
        std::vector<uint32_t>& scoreBuffer,
        size_t& tailPointer,
        size_t& headPointer)
{
    uint32_t nb = docBuffer.size() / BLOCK_SIZE;
    uint32_t res = docBuffer.size() % BLOCK_SIZE;

    if (pool_.reverse_)
    {
        size_t curPointer = UNDEFINED_POINTER;
        size_t lastPointer = UNDEFINED_POINTER;

        if (res > 0)
        {
            lastPointer = compressAndAppendBlock_(
                    &docBuffer[nb * BLOCK_SIZE],
                    &scoreBuffer[nb * BLOCK_SIZE],
                    res,
                    lastPointer,
                    tailPointer);

            if (curPointer == UNDEFINED_POINTER)
            {
                curPointer = lastPointer;
            }
        }

        for (int i = nb - 1; i >= 0; --i)
        {
            lastPointer = compressAndAppendBlock_(
                    &docBuffer[i * BLOCK_SIZE],
                    &scoreBuffer[i * BLOCK_SIZE],
                    BLOCK_SIZE,
                    lastPointer,
                    tailPointer);

            if (curPointer == UNDEFINED_POINTER)
            {
                curPointer = lastPointer;
            }
        }

        if (curPointer != UNDEFINED_POINTER)
        {
            headPointer = tailPointer = curPointer;
        }
    }
    else
    {
        for (uint32_t i = 0; i < nb; ++i)
        {
            tailPointer = compressAndAppendBlock_(
                    &docBuffer[i * BLOCK_SIZE],
                    &scoreBuffer[i * BLOCK_SIZE],
                    BLOCK_SIZE,
                    tailPointer,
                    UNDEFINED_POINTER);

            if (headPointer == UNDEFINED_POINTER)
            {
                headPointer = tailPointer;
            }
        }

        if (res > 0)
        {
            tailPointer = compressAndAppendBlock_(
                    &docBuffer[nb * BLOCK_SIZE],
                    &scoreBuffer[nb * BLOCK_SIZE],
                    res,
                    tailPointer,
                    UNDEFINED_POINTER);

            if (headPointer == UNDEFINED_POINTER)
            {
                headPointer = tailPointer;
            }
        }
    }

    docBuffer.clear();
    scoreBuffer.clear();
}

size_t AttrScoreInvertedIndex::compressAndAppendBlock_(
        uint32_t* docBlock,
        uint32_t* scoreBlock,
        uint32_t len,
        size_t lastPointer,
        size_t nextPointer)
{
    uint32_t maxDocId = pool_.reverse_ ? docBlock[0] : docBlock[len - 1];

    for (uint32_t i = len - 1; i > 0; --i)
    {
        docBlock[i] -= docBlock[i - 1];
    }

    if (pool_.reverse_)
    {
        std::reverse(docBlock, docBlock + len);
        std::reverse(scoreBlock, scoreBlock + len);
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docBlock[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
        memset(&scoreBlock[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
    }

    segment_[0] = len;

    size_t csize = BLOCK_SIZE * 2;
    memset(&segment_[2], 0, csize * sizeof(uint32_t));
    codec_.encodeArray(docBlock, BLOCK_SIZE, &segment_[2], csize);
    segment_[1] = csize;

    size_t scsize = BLOCK_SIZE * 2;
    memset(&segment_[csize + 3], 0, csize * sizeof(uint32_t));
    codec_.encodeArray(scoreBlock, BLOCK_SIZE, &segment_[csize + 3], scsize);
    segment_[csize + 2] = scsize;

    return pool_.appendSegment(segment_, maxDocId, csize + scsize + 3, lastPointer, nextPointer);
}

void AttrScoreInvertedIndex::retrieval(
        Algorithm algorithm,
        const std::vector<std::pair<std::string, int> >& term_list,
        const ZambeziFilter* filter,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    LOG(INFO) << "do retrievalWithBuffer ....";
    std::vector<std::pair<std::pair<uint32_t, int>, size_t> > queries;

    uint32_t minimumDf = 0xFFFFFFFF;
    bool hit_buffer = false;

    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getTermId(term_list[i].first);
        if (termid != INVALID_ID)
        {
            size_t pointer = pointers_.headPointers_.get(termid);
            if (pointer != UNDEFINED_POINTER)
            {
                queries.push_back(std::make_pair(std::make_pair(pointers_.df_.get(termid), term_list[i].second), pointer));
                minimumDf = std::min(queries.back().first.first, minimumDf);
            }
            else
            {
                const std::vector<uint32_t>& docBuffer = buffer_.docid_[termid];

                if (!hit_buffer)
                {
                    const std::vector<uint32_t>& scoreBuffer = buffer_.score_[termid];
                    docid_list.clear();
                    score_list.clear();
                    for (uint32_t j = 0; j < docBuffer.size(); ++j)
                    {
                        docid_list.push_back(docBuffer[j]);
                        score_list.push_back(scoreBuffer[j]);
                    }
                    hit_buffer = true;
                }
                else
                {
                    uint32_t iSet = 0, iCurrent = 0, iBuffer = 0;

                    while (true)
                    {
                        if (docBuffer[i] == docid_list[iCurrent])
                        {
                            docid_list[iSet] = docid_list[iCurrent];
                            score_list[iSet++] = score_list[iCurrent];

                            if (++iCurrent == docid_list.size()) break;
                            if (++iBuffer == docBuffer.size()) break;
                        }
                        else if (docBuffer[iBuffer] < docid_list[iCurrent])
                        {
                            if (++iBuffer == docBuffer.size()) break;
                        }
                        else
                        {
                            if (++iCurrent == docid_list.size()) break;
                        }
                    }

                    docid_list.resize(iSet);
                    score_list.resize(iSet);
                    if (iSet == 0) return;
                }
            }
        }
    }

    if (queries.empty() || queries.size() != term_list.size()) return;

    std::sort(queries.begin(), queries.end());

    std::vector<int> qScores(queries.size());
    std::vector<size_t> qHeadPointers(queries.size());

    for (uint32_t i = 0; i < queries.size(); ++i)
    {
        qScores[i] = queries[i].first.second;
        qHeadPointers[i] = queries[i].second;
    }

    intersectSvS_(qHeadPointers, qScores, filter, minimumDf, hits, docid_list, score_list);
}

uint32_t AttrScoreInvertedIndex::decompressDocidBlock_(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_.pool_[pSegment][pOffset + 6];
    size_t csize = pool_.pool_[pSegment][pOffset + 5];
    size_t nvalue = BLOCK_SIZE;
    codec.decodeArray(block, csize, outBlock, nvalue);

    uint32_t len = pool_.pool_[pSegment][pOffset + 4];
    if (pool_.reverse_)
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

uint32_t AttrScoreInvertedIndex::decompressScoreBlock_(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 5];
    const uint32_t* block = &pool_.pool_[pSegment][pOffset + csize + 7];
    size_t scsize = pool_.pool_[pSegment][pOffset + csize + 6];
    size_t nvalue = BLOCK_SIZE;
    codec.decodeArray(block, scsize, outBlock, nvalue);

    return pool_.pool_[pSegment][pOffset + 4];
}

bool AttrScoreInvertedIndex::gallopSearch_(
        FastPFor& codec,
        uint32_t* blockDocid,
        uint32_t* blockScore,
        uint32_t& count,
        uint32_t& index,
        size_t& pointer,
        uint32_t pivot) const
{
    if (LESS_THAN(blockDocid[count - 1], pivot, pool_.reverse_))
    {
        if ((pointer = pool_.nextPointer(pointer, pivot)) == UNDEFINED_POINTER)
            return false;

        count = decompressDocidBlock_(codec, &blockDocid[0], pointer);
        decompressScoreBlock_(codec, &blockScore[0], pointer);
        index = 0;
    }

    if (GREATER_THAN_EQUAL(blockDocid[index], pivot, pool_.reverse_))
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
    while ((uint32_t)tempIndex < count && LESS_THAN_EQUAL(blockDocid[tempIndex], pivot, pool_.reverse_))
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
    while (tempIndex >= 0 && GREATER_THAN(blockDocid[tempIndex], pivot, pool_.reverse_))
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

        if (LESS_THAN(pivot, blockDocid[mid], pool_.reverse_))
        {
            endIndex = mid;
        }
        else if (GREATER_THAN(pivot, blockDocid[mid], pool_.reverse_))
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

void AttrScoreInvertedIndex::intersectPostingsLists_(
        FastPFor& codec,
        const ZambeziFilter* filter,
        size_t pointer0,
        size_t pointer1,
        int weight0,
        int weight1,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    uint32_t blockDocid0[BLOCK_SIZE];
    uint32_t blockDocid1[BLOCK_SIZE];
    uint32_t blockScore0[BLOCK_SIZE];
    uint32_t blockScore1[BLOCK_SIZE];

    uint32_t c0 = decompressDocidBlock_(codec, &blockDocid0[0], pointer0), i0 = 0;
    uint32_t c1 = decompressDocidBlock_(codec, &blockDocid1[0], pointer1), i1 = 0;
    decompressScoreBlock_(codec, &blockScore0[0], pointer0);
    decompressScoreBlock_(codec, &blockScore1[0], pointer1);

    uint32_t next = filter->find_first();

    while (true)
    {
        if (blockDocid0[i0] == blockDocid1[i1])
        {
            if (filter->test(blockDocid0[i0]))
            {
                docid_list.push_back(blockDocid0[i0]);
                score_list.push_back(blockScore0[i0] * weight0 + blockScore1[i1] * weight1);
            }

            if ((next = filter->find_next(blockDocid0[i0] + 1)) == INVALID_ID)
                break;

            if (!gallopSearch_(codec, blockDocid0, blockScore0, c0, ++i0, pointer0, next))
                break;

            if (!gallopSearch_(codec, blockDocid1, blockScore1, c1, ++i1, pointer1, next))
                break;
        }
        else if (LESS_THAN(blockDocid0[i0], blockDocid1[i1], pool_.reverse_))
        {
            if ((next = filter->find_next(blockDocid0[i0])) == INVALID_ID)
                break;

            if (!gallopSearch_(codec, blockDocid0, blockScore0, c0, ++i0, pointer0, next))
                break;
        }
        else
        {
            if ((next = filter->find_next(blockDocid1[i1])) == INVALID_ID)
                break;

            if (!gallopSearch_(codec, blockDocid1, blockScore1, c1, ++i1, pointer1, next))
                break;
        }
    }
}

void AttrScoreInvertedIndex::intersectSetPostingsList_(
        FastPFor& codec,
        size_t pointer,
        int weight,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    uint32_t blockDocid[BLOCK_SIZE];
    uint32_t blockScore[BLOCK_SIZE];
    uint32_t c = decompressDocidBlock_(codec, &blockDocid[0], pointer);
    decompressScoreBlock_(codec, &blockScore[0], pointer);
    uint32_t iSet = 0, iCurrent = 0, i = 0;

    while (iCurrent < docid_list.size())
    {
        if (blockDocid[i] == docid_list[iCurrent])
        {
            docid_list[iSet] = docid_list[iCurrent];
            score_list[iSet++] = score_list[iCurrent] + blockScore[i] * weight;

            if (++iCurrent == docid_list.size())
                break;

            if (++i == c)
            {
                if ((pointer = pool_.nextPointer(pointer)) == UNDEFINED_POINTER)
                    break;

                c = decompressDocidBlock_(codec, &blockDocid[0], pointer);
                decompressScoreBlock_(codec, &blockScore[0], pointer);
                i = 0;
            }
        }
        else if (LESS_THAN(blockDocid[i], docid_list[iCurrent], pool_.reverse_))
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
            while (LESS_THAN(docid_list[iCurrent], blockDocid[i], pool_.reverse_));
        }
    }

    docid_list.resize(iSet);
    score_list.resize(iSet);
}

void AttrScoreInvertedIndex::intersectSvS_(
        std::vector<size_t>& headPointers,
        const std::vector<int>& qScores,
        const ZambeziFilter* filter,
        uint32_t minDf,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    FastPFor codec;

    if (headPointers.size() < 2)
    {
        uint32_t block[BLOCK_SIZE];
        uint32_t sblock[BLOCK_SIZE];
        uint32_t length = std::min(minDf, hits);

        docid_list.reserve(length);
        score_list.reserve(length);

        size_t t = headPointers[0];
        uint32_t next = filter->find_first();
        uint32_t c = decompressDocidBlock_(codec, &block[0], t);
        uint32_t i = 0;
        decompressScoreBlock_(codec, &sblock[0], t);

        while (gallopSearch_(codec, block, sblock, c, i, t, next))
        {
            if (block[i] == next)
            {
                docid_list.push_back(block[i]);
                score_list.push_back(sblock[i]);
                if (docid_list.size() == length) break;

                if ((next = filter->find_next(block[i] + 1)) == INVALID_ID)
                    break;
            }
            else
            {
                if ((next = filter->find_next(block[i])) == INVALID_ID)
                    break;
            }
        }

        return;
    }

    docid_list.reserve(minDf);
    score_list.reserve(minDf);

    intersectPostingsLists_(codec, filter, headPointers[0], headPointers[1], qScores[0], qScores[1], docid_list, score_list);

    for (uint32_t i = 2; i < headPointers.size(); ++i)
    {
        if (docid_list.empty()) return;
        intersectSetPostingsList_(codec, headPointers[i], qScores[i], docid_list, score_list);
    }

    if (hits < docid_list.size())
    {
        docid_list.resize(hits);
        score_list.resize(hits);
    }
}

}

NS_IZENELIB_IR_END
