#include <ir/Zambezi/AttrScoreInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <glog/logging.h>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace
{

template <class BlockType>
uint32_t linearSearch(
        const BlockType &block,
        bool reverse,
        uint32_t count,
        uint32_t index,
        uint32_t pivot)
{
    if (index >= count || LESS_THAN((uint32_t)block[count - 1], pivot, reverse))
        return INVALID_ID;

    for (;; index += 4)
    {
        if (GREATER_THAN_EQUAL((uint32_t)block[index], pivot, reverse)) return index;
        if (GREATER_THAN_EQUAL((uint32_t)block[index + 1], pivot, reverse)) return index + 1;
        if (GREATER_THAN_EQUAL((uint32_t)block[index + 2], pivot, reverse)) return index + 2;
        if (GREATER_THAN_EQUAL((uint32_t)block[index + 3], pivot, reverse)) return index + 3;
    }

    assert(false);
    return INVALID_ID;
}

template <class BlockType>
uint32_t gallopSearch(
        const BlockType& block,
        bool reverse,
        uint32_t count,
        uint32_t index,
        uint32_t pivot)
{
    if (index >= count || LESS_THAN((uint32_t)block[count - 1], pivot, reverse))
        return INVALID_ID;

    if (GREATER_THAN_EQUAL((uint32_t)block[index], pivot, reverse))
        return index;

    if ((uint32_t)block[count - 1] == pivot)
        return count - 1;

    int beginIndex = index;
    int hop = 1;
    int tempIndex = beginIndex + 1;
    while ((uint32_t)tempIndex < count && LESS_THAN_EQUAL((uint32_t)block[tempIndex], pivot, reverse))
    {
        beginIndex = tempIndex;
        tempIndex += hop;
        hop *= 2;
    }

    if ((uint32_t)block[beginIndex] == pivot)
        return beginIndex;

    int endIndex = count - 1;
    hop = 1;
    tempIndex = endIndex - 1;
    while (tempIndex >= 0 && GREATER_THAN((uint32_t)block[tempIndex], pivot, reverse))
    {
        endIndex = tempIndex;
        tempIndex -= hop;
        hop *= 2;
    }

    if ((uint32_t)block[endIndex] == pivot)
        return endIndex;

    // Binary search between begin and end indexes
    while (beginIndex < endIndex)
    {
        uint32_t mid = beginIndex + (endIndex - beginIndex) / 2;

        if (GREATER_THAN((uint32_t)block[mid], pivot, reverse))
        {
            endIndex = mid;
        }
        else if (LESS_THAN((uint32_t)block[mid], pivot, reverse))
        {
            beginIndex = mid + 1;
        }
        else
        {
            return mid;
        }
    }

    return endIndex;
}

}

AttrScoreInvertedIndex::AttrScoreInvertedIndex(
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        uint32_t vocabSize,
        bool reverse)
    : buffer_(vocabSize)
    , pool_(maxPoolSize, numberOfPools, reverse)
    , dictionary_(vocabSize)
    , pointers_(vocabSize, 0)
    , segment_(getAlignedIntArray(BUFFER_SIZE, 16))
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
    return pointers_.totalDocs;
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

        boost::shared_ptr<AttrScoreBufferMaps::PostingType>& posting = buffer_.buffer[id];

        if (pointers_.df.get(id) <= DF_CUTOFF)
        {
            if (!posting)
            {
                buffer_.resetBuffer(id, DF_CUTOFF);
            }
            posting->push_back(AttrScoreBufferMaps::ElemType(docid, score_list[i]));
            pointers_.df.increment(id);
            pointers_.cf.increment(id);

            continue;
        }

        if (posting->capacity() < BLOCK_SIZE)
        {
            buffer_.resetBuffer(id, BLOCK_SIZE);
        }

        posting->push_back(AttrScoreBufferMaps::ElemType(docid, score_list[i]));

        pointers_.df.increment(id);
        pointers_.cf.increment(id);

        if (posting->size() == posting->capacity())
        {
            processTermBuffer_(
                    *posting,
                    buffer_.tailPointer[id],
                    pointers_.headPointers.get(id));

            uint32_t newLen = posting->capacity() * (posting->capacity() < MAX_BLOCK_SIZE ? EXPANSION_RATE : 1);
            buffer_.resetBuffer(id, newLen, false);
        }
    }
    ++pointers_.totalDocs;
}

void AttrScoreInvertedIndex::flush()
{
    uint32_t term = INVALID_ID;
    while ((term = buffer_.nextIndex(term, DF_CUTOFF)) != INVALID_ID)
    {
        processTermBuffer_(
                *buffer_.buffer[term],
                buffer_.tailPointer[term],
                pointers_.headPointers.get(term));

        buffer_.resetBuffer(term, buffer_.buffer[term]->capacity(), false);
    }
}

void AttrScoreInvertedIndex::processTermBuffer_(
        AttrScoreBufferMaps::PostingType& posting,
        size_t& tailPointer,
        size_t& headPointer)
{
    uint32_t nb = posting.size() / BP_BLOCK_SIZE;
    uint32_t res = posting.size() % BP_BLOCK_SIZE;

    size_t capacity = BP_BLOCK_SIZE * (nb + (res ? 1 : 0));

    boost::shared_array<uint32_t> docBuffer(getAlignedIntArray(capacity, 16));
    boost::shared_array<uint32_t> scoreBuffer(getAlignedIntArray(capacity, 16));

    for (size_t i = 0; i < posting.size(); ++i)
    {
        docBuffer[i] = posting[i].docid;
        scoreBuffer[i] = posting[i].score;
    }

    if (pool_.reverse_)
    {
        size_t curPointer = UNDEFINED_POINTER;
        size_t lastPointer = UNDEFINED_POINTER;

        if (res > 0)
        {
            lastPointer = compressAndAppendBlock_(
                    &docBuffer[nb * BP_BLOCK_SIZE],
                    &scoreBuffer[nb * BP_BLOCK_SIZE],
                    res,
                    lastPointer,
                    tailPointer);

            curPointer = lastPointer;
        }

        for (int i = nb - 1; i >= 0; --i)
        {
            lastPointer = compressAndAppendBlock_(
                    &docBuffer[i * BP_BLOCK_SIZE],
                    &scoreBuffer[i * BP_BLOCK_SIZE],
                    BP_BLOCK_SIZE,
                    lastPointer,
                    tailPointer);

            if (curPointer == UNDEFINED_POINTER)
            {
                curPointer = lastPointer;
            }
        }

        headPointer = tailPointer = curPointer;
    }
    else
    {
        for (uint32_t i = 0; i < nb; ++i)
        {
            tailPointer = compressAndAppendBlock_(
                    &docBuffer[i * BP_BLOCK_SIZE],
                    &scoreBuffer[i * BP_BLOCK_SIZE],
                    BP_BLOCK_SIZE,
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
                    &docBuffer[nb * BP_BLOCK_SIZE],
                    &scoreBuffer[nb * BP_BLOCK_SIZE],
                    res,
                    tailPointer,
                    UNDEFINED_POINTER);


            if (headPointer == UNDEFINED_POINTER)
            {
                headPointer = tailPointer;
            }
        }
    }
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

    if (len < BP_BLOCK_SIZE)
    {
        memset(&docBlock[len], 0, (BP_BLOCK_SIZE - len) * sizeof(docBlock[0]));
        memset(&scoreBlock[len], 0, (BP_BLOCK_SIZE - len) * sizeof(scoreBlock[0]));
    }

    segment_[0] = len;

    size_t csize = len;
    memset(&segment_[4], 0, BP_BLOCK_SIZE * sizeof(segment_[0]));
    codec_.encodeArray(docBlock, BP_BLOCK_SIZE, &segment_[4], csize);
    segment_[1] = csize;
    segment_[2] = csize = (csize + 3) / 4 * 4;

    size_t scsize = len;
    memset(&segment_[csize + 4], 0, BP_BLOCK_SIZE * sizeof(segment_[0]));
    codec_.encodeArray(scoreBlock, BP_BLOCK_SIZE, &segment_[csize + 4], scsize);
    segment_[3] = scsize;
    scsize = (scsize + 3) / 4 * 4;

    return pool_.appendSegment(&segment_[0], maxDocId, csize + scsize + 4, lastPointer, nextPointer);
}

uint32_t AttrScoreInvertedIndex::decompressDocidBlock_(
        SIMDBinaryPacking& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_.pool_[pSegment][pOffset + 8];
    size_t csize = pool_.pool_[pSegment][pOffset + 5];
    size_t nvalue = BP_BLOCK_SIZE;
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
        SIMDBinaryPacking& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    const uint32_t* block = &pool_.pool_[pSegment][pOffset + csize + 8];
    size_t scsize = pool_.pool_[pSegment][pOffset + 7];
    size_t nvalue = BP_BLOCK_SIZE;
    codec.decodeArray(block, scsize, outBlock, nvalue);

    return pool_.pool_[pSegment][pOffset + 4];
}

void AttrScoreInvertedIndex::retrieve(
        Algorithm algorithm,
        const std::vector<std::pair<std::string, int> >& term_list,
        const FilterBase* filter,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    std::vector<std::pair<std::pair<uint32_t, uint32_t>, int> > queries;

    uint32_t minimumDf = 0xFFFFFFFF;

    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getTermId(term_list[i].first);
        if (termid == INVALID_ID) return;

        uint32_t df = pointers_.df.get(termid);
        queries.push_back(std::make_pair(std::make_pair(df, termid), term_list[i].second));
        minimumDf = std::min(df, minimumDf);
    }

    std::sort(queries.begin(), queries.end());

    std::vector<uint32_t> qTerms(queries.size());
    std::vector<int> qScores(queries.size());

    for (uint32_t i = 0; i < queries.size(); ++i)
    {
        qTerms[i] = queries[i].first.second;
        qScores[i] = queries[i].second;
    }

    if (algorithm == SVS)
    {
        intersectSvS_(qTerms, qScores, filter, minimumDf, hits, docid_list, score_list);
    }
}

bool AttrScoreInvertedIndex::unionIterate_(
        SIMDBinaryPacking& codec,
        bool& in_buffer,
        const boost::shared_ptr<AttrScoreBufferMaps::PostingType>& buffer,
        uint32_t* docid_seg,
        uint32_t* score_seg,
        uint32_t pivot,
        uint32_t& count,
        uint32_t& index,
        size_t& pointer,
        uint32_t& docid,
        uint32_t& score) const
{
    if (in_buffer)
    {
        index = pool_.reverse_
            ? linearSearch(buffer->rend() - count, pool_.reverse_, count, index, pivot)
            : linearSearch(buffer->begin(), pool_.reverse_, count, index, pivot);
        if (index == INVALID_ID)
        {
            in_buffer = false;
            if ((pointer = pool_.nextPointer(pointer, pivot)) == UNDEFINED_POINTER)
                return false;

            count = decompressDocidBlock_(codec, docid_seg, pointer);
            decompressScoreBlock_(codec, score_seg, pointer);
            index = linearSearch(docid_seg, pool_.reverse_, count, 0, pivot);

            docid = docid_seg[index];
            score = score_seg[index];
        }
        else
        {
            if (pool_.reverse_)
            {
                docid = (*buffer)[count - 1 - index].docid;
                score = (*buffer)[count - 1 - index].score;
            }
            else
            {
                docid = (*buffer)[index].docid;
                score = (*buffer)[index].score;
            }
        }
    }
    else
    {
        if (count == 0 || LESS_THAN(docid_seg[count - 1], pivot, pool_.reverse_))
        {
            if ((pointer = pool_.nextPointer(pointer, pivot)) == UNDEFINED_POINTER)
            {
                in_buffer = true;
                count = buffer->size();
                index = pool_.reverse_
                    ? linearSearch(buffer->rend() - count, pool_.reverse_, count, 0, pivot)
                    : linearSearch(buffer->begin(), pool_.reverse_, count, 0, pivot);

                if (index == INVALID_ID) return false;

                if (pool_.reverse_)
                {
                    docid = (*buffer)[count - 1 - index].docid;
                    score = (*buffer)[count - 1 - index].score;
                }
                else
                {
                    docid = (*buffer)[index].docid;
                    score = (*buffer)[index].score;
                }

                return true;
            }

            count = decompressDocidBlock_(codec, docid_seg, pointer);
            decompressScoreBlock_(codec, score_seg, pointer);
            index = 0;
        }

        index = linearSearch(docid_seg, pool_.reverse_, count, index, pivot);

        docid = docid_seg[index];
        score = score_seg[index];
    }

    return true;
}

void AttrScoreInvertedIndex::intersectPostingsLists_(
        SIMDBinaryPacking& codec,
        const FilterBase* filter,
        uint32_t term0,
        uint32_t term1,
        int weight0,
        int weight1,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list,
        uint32_t hits) const
{
    boost::shared_array<uint32_t> blockDocid0(getAlignedIntArray(BP_BLOCK_SIZE, 16));
    boost::shared_array<uint32_t> blockDocid1(getAlignedIntArray(BP_BLOCK_SIZE, 16));
    boost::shared_array<uint32_t> blockScore0(getAlignedIntArray(BP_BLOCK_SIZE, 16));
    boost::shared_array<uint32_t> blockScore1(getAlignedIntArray(BP_BLOCK_SIZE, 16));

    size_t pointer0 = pointers_.headPointers.get(term0);
    size_t pointer1 = pointers_.headPointers.get(term1);

    boost::shared_ptr<AttrScoreBufferMaps::PostingType> buffer0(buffer_.getBuffer(term0));
    boost::shared_ptr<AttrScoreBufferMaps::PostingType> buffer1(buffer_.getBuffer(term1));

    uint32_t c0 = pool_.reverse_ ? buffer0->size() : 0, c1 = pool_.reverse_ ? buffer1->size() : 0;
    uint32_t i0 = 0, i1 = 0;

    uint32_t id0 = 0, id1 = 0;
    uint32_t sc0 = 0, sc1 = 0;

    bool in_buffer0 = pool_.reverse_, in_buffer1 = pool_.reverse_;
    uint32_t eligible = filter->find_first(pool_.reverse_);

    if (!unionIterate_(codec, in_buffer0, buffer0, &blockDocid0[0], &blockScore0[0], eligible, c0, i0, pointer0, id0, sc0))
        return;

    if ((eligible = filter->test(id0) ? id0 : filter->find_next(id0, pool_.reverse_)) == INVALID_ID)
        return;

    if (!unionIterate_(codec, in_buffer1, buffer1, &blockDocid1[0], &blockScore1[0], eligible, c1, i1, pointer1, id1, sc1))
        return;

    while (true)
    {
        if (id0 == id1)
        {
            docid_list.push_back(id0);
            score_list.push_back(sc0 * weight0 + sc1 * weight1);
            if (score_list.size() == hits)
                break;

            if ((eligible = filter->find_next(id0, pool_.reverse_)) == INVALID_ID)
                break;

            if (!unionIterate_(codec, in_buffer0, buffer0, &blockDocid0[0], &blockScore0[0], eligible, c0, i0, pointer0, id0, sc0))
                return;

            if ((eligible = filter->test(id0) ? id0 : filter->find_next(id0, pool_.reverse_)) == INVALID_ID)
                return;

            if (!unionIterate_(codec, in_buffer1, buffer1, &blockDocid1[0], &blockScore1[0], eligible, c1, i1, pointer1, id1, sc1))
                return;
        }
        else if (LESS_THAN(id0, id1, pool_.reverse_))
        {
            if ((eligible = filter->test(id1) ? id1 : filter->find_next(id1, pool_.reverse_)) == INVALID_ID)
                break;

            if (!unionIterate_(codec, in_buffer0, buffer0, &blockDocid0[0], &blockScore0[0], eligible, c0, i0, pointer0, id0, sc0))
                return;
        }
        else
        {
            if ((eligible = filter->test(id0) ? id0 : filter->find_next(id0, pool_.reverse_)) == INVALID_ID)
                break;

            if (!unionIterate_(codec, in_buffer1, buffer1, &blockDocid1[0], &blockScore1[0], eligible, c1, i1, pointer1, id1, sc1))
                return;
        }
    }
}

void AttrScoreInvertedIndex::intersectSetPostingsList_(
        SIMDBinaryPacking& codec,
        uint32_t term,
        int weight,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    uint32_t iSet = 0, iCurrent = 0;

    size_t pointer = pointers_.headPointers.get(term);
    boost::shared_array<uint32_t> blockDocid(getAlignedIntArray(BP_BLOCK_SIZE, 16));
    boost::shared_array<uint32_t> blockScore(getAlignedIntArray(BP_BLOCK_SIZE, 16));

    boost::shared_ptr<AttrScoreBufferMaps::PostingType> buffer(buffer_.getBuffer(term));

    uint32_t c = pool_.reverse_ ? buffer->size() : 0, i = 0;
    uint32_t id = 0, sc = 0;

    bool in_buffer = pool_.reverse_;

    if (!unionIterate_(codec, in_buffer, buffer, &blockDocid[0], &blockScore[0], docid_list[iCurrent], c, i, pointer, id, sc))
        return;

    if ((iCurrent = linearSearch(docid_list, pool_.reverse_, docid_list.size(), iCurrent, id)) == INVALID_ID)
        return;

    while (true)
    {
        if (id == docid_list[iCurrent])
        {
            docid_list[iSet] = docid_list[iCurrent];
            score_list[iSet++] = score_list[iCurrent++] + sc * weight;

            if (iCurrent == docid_list.size())
                break;
        }

        if (!unionIterate_(codec, in_buffer, buffer, &blockDocid[0], &blockScore[0], docid_list[iCurrent], c, i, pointer, id, sc))
            break;

        if ((iCurrent = linearSearch(docid_list, pool_.reverse_, docid_list.size(), iCurrent, id)) == INVALID_ID)
            break;
    }

    docid_list.resize(iSet);
    score_list.resize(iSet);
}

void AttrScoreInvertedIndex::intersectSvS_(
        const std::vector<uint32_t>& qTerms,
        const std::vector<int>& qScores,
        const FilterBase* filter,
        uint32_t minDf,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    SIMDBinaryPacking codec;

    if (qTerms.size() == 1)
    {
        boost::shared_array<uint32_t> blockDocid(getAlignedIntArray(BP_BLOCK_SIZE, 16));
        boost::shared_array<uint32_t> blockScore(getAlignedIntArray(BP_BLOCK_SIZE, 16));
        uint32_t length = std::min(minDf, hits);

        docid_list.reserve(length);
        score_list.reserve(length);

        boost::shared_ptr<AttrScoreBufferMaps::PostingType> buffer(buffer_.getBuffer(qTerms[0]));
        size_t pointer = pointers_.headPointers.get(qTerms[0]);
        uint32_t c = pool_.reverse_ ? buffer->size() : 0, i = 0;
        uint32_t id = 0, sc = 0;

        bool in_buffer = pool_.reverse_;
        uint32_t eligible = filter->find_first(pool_.reverse_);

        if (!unionIterate_(codec, in_buffer, buffer, &blockDocid[0], &blockScore[0], eligible, c, i, pointer, id, sc))
            return;

        if ((eligible = filter->test(id) ? id : filter->find_next(id, pool_.reverse_)) == INVALID_ID)
            return;

        while (true)
        {
            if (id == eligible)
            {
                docid_list.push_back(id);
                score_list.push_back(sc);

                if (docid_list.size() == length)
                    break;

                if ((eligible = filter->find_next(eligible, pool_.reverse_)) == INVALID_ID)
                    break;
            }

            if (!unionIterate_(codec, in_buffer, buffer, &blockDocid[0], &blockScore[0], eligible, c, i, pointer, id, sc))
                break;

            if ((eligible = filter->test(id) ? id : filter->find_next(id, pool_.reverse_)) == INVALID_ID)
                break;
        }

        return;
    }

    docid_list.reserve(minDf);
    score_list.reserve(minDf);

    intersectPostingsLists_(codec, filter, qTerms[0], qTerms[1], qScores[0], qScores[1], docid_list, score_list, hits);

    for (uint32_t i = 2; i < qTerms.size(); ++i)
    {
        if (docid_list.empty()) return;
        intersectSetPostingsList_(codec, qTerms[i], qScores[i], docid_list, score_list);
    }

    if (hits < docid_list.size())
    {
        docid_list.resize(hits);
        score_list.resize(hits);
    }
}

}

NS_IZENELIB_IR_END
