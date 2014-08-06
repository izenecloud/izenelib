#include <ir/Zambezi/AttrScoreInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>
#include <ir/Zambezi/search/LinearSearch.hpp>
//#include <ir/Zambezi/search/GallopSearch.hpp>

#include <util/compression/simd-compression/compositecodec.h>
#include <util/compression/simd-compression/simdbinarypacking.h>
#include <util/compression/simd-compression/variablebyte.h>

#include <glog/logging.h>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

using namespace SIMDCompression;

typedef CompositeCodec<SIMDBinaryPacking<SIMDIntegratedBlockPacker<CoarseDelta4SIMD, true> >, VariableByte<true> > ForwardCodecType;
typedef CompositeCodec<SIMDBinaryPacking<SIMDIntegratedBlockPacker<ReverseCoarseDelta4SIMD, true> >, VariableByte<true> > ReverseCodecType;
typedef CompositeCodec<SIMDBinaryPacking<SIMDBlockPacker<NoDelta, true> >, VariableByte<false> > NoDeltaCodecType;

static ForwardCodecType forwardCodec;
static ReverseCodecType reverseCodec;
static NoDeltaCodecType noDeltaCodec;

AttrScoreInvertedIndex::AttrScoreInvertedIndex(
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        uint32_t vocabSize,
        bool reverse)
    : buffer_(vocabSize)
    , pool_(maxPoolSize, numberOfPools)
    , dictionary_(vocabSize)
    , pointers_(vocabSize, 0)
    , reverse_(reverse)
{
}

AttrScoreInvertedIndex::~AttrScoreInvertedIndex()
{
}

void AttrScoreInvertedIndex::save(std::ostream& ostr) const
{
    LOG(INFO) << "Save: start....";

    ostr.write((const char*)&reverse_, sizeof(reverse_));

    std::streamoff offset = ostr.tellp();
    buffer_.save(ostr, reverse_);
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

    istr.read((char*)&reverse_, sizeof(reverse_));

    std::streamoff offset = istr.tellg();
    buffer_.load(istr, reverse_);
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

        boost::shared_array<uint32_t>& posting = buffer_.buffer[id];

        if (!posting)
        {
            buffer_.resetBuffer(id, DF_CUTOFF, reverse_, false);
        }

        if (reverse_)
        {
            posting[3 + posting[0] - posting[1]] = docid;
            posting[3 + 2 * posting[0] - posting[1]] = score_list[i];
        }
        else
        {
            posting[4 + posting[1]] = docid;
            posting[4 + posting[0] + posting[1]] = score_list[i];
        }
        ++posting[1];

        pointers_.df.increment(id);
        pointers_.cf.increment(id);

        if (posting[0] == posting[1])
        {
            if (posting[0] < BLOCK_SIZE)
            {
                buffer_.resetBuffer(id, posting[0] * EXPANSION_RATE, reverse_, true);
            }
            else
            {
                processTermBuffer_(
                        posting,
                        buffer_.tailPointer[id],
                        pointers_.headPointers.get(id));

                uint32_t newLen = posting[0] * (posting[0] < MAX_BLOCK_SIZE ? EXPANSION_RATE : 1);
                buffer_.resetBuffer(id, newLen, reverse_, false);
            }
        }
    }
    ++pointers_.totalDocs;
}

void AttrScoreInvertedIndex::flush()
{
//  uint32_t term = INVALID_ID;
//  while ((term = buffer_.nextIndex(term, DF_CUTOFF)) != INVALID_ID)
//  {
//      processTermBuffer_(
//              buffer_.buffer[term],
//              buffer_.tailPointer[term],
//              pointers_.headPointers.get(term));

//      buffer_.resetBuffer(term, buffer_.buffer[term][0], reverse_, false);
//  }
}

void AttrScoreInvertedIndex::processTermBuffer_(
        boost::shared_array<uint32_t>& posting,
        size_t& tailPointer,
        size_t& headPointer)
{
    uint32_t capacity = posting[0];
    uint32_t size = posting[1];

    uint32_t nb = size / BP_BLOCK_SIZE;
    uint32_t res = size % BP_BLOCK_SIZE;

    if (reverse_)
    {
        size_t curPointer = UNDEFINED_POINTER;
        size_t lastPointer = UNDEFINED_POINTER;

        if (res > 0)
        {
            lastPointer = compressAndAppendBlock_(
                    &posting[4 + capacity - size],
                    &posting[4 + capacity * 2 - size],
                    res,
                    lastPointer,
                    tailPointer);

            curPointer = lastPointer;
        }

        for (uint32_t i = nb; i > 0; --i)
        {
            lastPointer = compressAndAppendBlock_(
                    &posting[4 + capacity - i * BP_BLOCK_SIZE],
                    &posting[4 + capacity * 2 - i * BP_BLOCK_SIZE],
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
                    &posting[4 + i * BP_BLOCK_SIZE],
                    &posting[4 + capacity + i * BP_BLOCK_SIZE],
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
                    &posting[4 + nb * BP_BLOCK_SIZE],
                    &posting[4 + capacity + nb * BP_BLOCK_SIZE],
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
    uint32_t maxDocId = docBlock[len - 1];

    segment_[0] = len;

    size_t csize = len;
    memset(&segment_[4], 0, len * sizeof(segment_[0]));
    if (reverse_)
        reverseCodec.encodeArray(docBlock, len, &segment_[4], csize);
    else
        forwardCodec.encodeArray(docBlock, len, &segment_[4], csize);
    segment_[1] = csize;
    segment_[2] = csize = (csize + 3) / 4 * 4;

    size_t scsize = len;
    memset(&segment_[csize + 4], 0, len * sizeof(segment_[0]));
    noDeltaCodec.encodeArray(scoreBlock, len, &segment_[csize + 4], scsize);
    segment_[3] = scsize;
    scsize = (scsize + 3) / 4 * 4;

    return pool_.appendSegment(&segment_[0], maxDocId, csize + scsize + 4, lastPointer, nextPointer);
}

uint32_t AttrScoreInvertedIndex::decompressDocidBlock_(
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_.pool_[pSegment][pOffset + 8];
    size_t csize = pool_.pool_[pSegment][pOffset + 5];
    size_t nvalue = BP_BLOCK_SIZE;
    if (reverse_)
        reverseCodec.decodeArray(block, csize, outBlock, nvalue);
    else
        forwardCodec.decodeArray(block, csize, outBlock, nvalue);

    return pool_.pool_[pSegment][pOffset + 4];
}

uint32_t AttrScoreInvertedIndex::decompressScoreBlock_(
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    const uint32_t* block = &pool_.pool_[pSegment][pOffset + csize + 8];
    size_t scsize = pool_.pool_[pSegment][pOffset + 7];
    size_t nvalue = BP_BLOCK_SIZE;
    noDeltaCodec.decodeArray(block, scsize, outBlock, nvalue);

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
    if (term_list.empty()) return;

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
        bool& in_buffer,
        const uint32_t* buffer,
        uint32_t* docid_seg,
        uint32_t* score_seg,
        uint32_t pivot,
        uint32_t& count,
        uint32_t& tail,
        uint32_t& index,
        size_t& pointer,
        uint32_t& docid,
        uint32_t& score) const
{
    if (in_buffer)
    {
        if (index >= count || LESS_THAN(tail, pivot, reverse_)
                || (index = linearSearch(buffer, reverse_, count, index, pivot)) == INVALID_ID)
        {
            if (!reverse_) return false;

            if ((pointer = pool_.nextPointer(pointer, pivot, reverse_)) == UNDEFINED_POINTER)
                return false;

            in_buffer = false;

            count = decompressDocidBlock_(docid_seg, pointer);
            tail = docid_seg[count - 1];
            decompressScoreBlock_(score_seg, pointer);
            index = linearSearch(docid_seg, reverse_, count, 0, pivot);

            docid = docid_seg[index];
            score = score_seg[index];
        }
        else
        {
            docid = buffer[index];
            score = buffer[(reverse_ ? count : buffer[-4]) + index];
        }
    }
    else
    {
        if (index >= count || LESS_THAN(tail, pivot, reverse_)
                || (index = linearSearch(docid_seg, reverse_, count, index, pivot)) == INVALID_ID)
        {
            if ((pointer = pool_.nextPointer(pointer, pivot, reverse_)) == UNDEFINED_POINTER)
            {
                if (reverse_) return false;

                count = buffer[-3];
                tail = buffer[count - 1];

                if (count == 0 || LESS_THAN(tail, pivot, reverse_)
                        || (index = linearSearch(buffer, reverse_, count, 0, pivot)) == INVALID_ID)
                    return false;

                in_buffer = true;

                docid = buffer[index];
                score = buffer[buffer[-4] + index];

                return true;
            }

            count = decompressDocidBlock_(docid_seg, pointer);
            tail = docid_seg[count - 1];
            decompressScoreBlock_(score_seg, pointer);
            index = linearSearch(docid_seg, reverse_, count, 0, pivot);
        }

        docid = docid_seg[index];
        score = score_seg[index];
    }

    return true;
}

void AttrScoreInvertedIndex::intersectPostingsLists_(
        const FilterBase* filter,
        uint32_t term0,
        uint32_t term1,
        int weight0,
        int weight1,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list,
        uint32_t hits) const
{
    uint32_t blockDocid0[BP_BLOCK_SIZE + 15] __attribute__((aligned(16)));
    uint32_t blockDocid1[BP_BLOCK_SIZE + 15] __attribute__((aligned(16)));
    uint32_t blockScore0[BP_BLOCK_SIZE] __attribute__((aligned(16)));
    uint32_t blockScore1[BP_BLOCK_SIZE] __attribute__((aligned(16)));

    size_t pointer0 = pointers_.headPointers.get(term0);
    size_t pointer1 = pointers_.headPointers.get(term1);

    boost::shared_array<uint32_t> buffer0(buffer_.getBuffer(term0));
    boost::shared_array<uint32_t> buffer1(buffer_.getBuffer(term1));

    const uint32_t* docBuffer0 = &buffer0[4];
    const uint32_t* docBuffer1 = &buffer1[4];

    uint32_t c0 = reverse_ ? buffer0[0] : 0, c1 = reverse_ ? buffer1[0] : 0;
    uint32_t t0 = reverse_ ? buffer0[c0 + 3] : 0, t1 = reverse_ ? buffer1[c1 + 3] : 0;
    uint32_t i0 = reverse_ ? buffer0[0] - buffer0[1] : 0, i1 = reverse_ ? buffer1[0] - buffer1[1] : 0;

    uint32_t id0 = 0, id1 = 0;
    uint32_t sc0 = 0, sc1 = 0;

    bool in_buffer0 = reverse_, in_buffer1 = reverse_;

    if ((id1 = filter->find_first(reverse_)) == INVALID_ID) return;

    if (!unionIterate_(in_buffer0, docBuffer0, blockDocid0, blockScore0, id1, c0, t0, i0, pointer0, id0, sc0))
        return;

    if (!unionIterate_(in_buffer1, docBuffer1, blockDocid1, blockScore1, id0, c1, t1, i1, pointer1, id1, sc1))
        return;

    while (true)
    {
        if (id0 == id1)
        {
            if (filter->test(id0))
            {
                docid_list.push_back(id0);
                score_list.push_back(sc0 * weight0 + sc1 * weight1);

                if (docid_list.size() == hits) break;
            }

            if ((id1 = filter->find_next(id0, reverse_)) == INVALID_ID)
                break;

            if (!unionIterate_(in_buffer0, docBuffer0, blockDocid0, blockScore0, id1, c0, t0, i0, pointer0, id0, sc0))
                return;

            if (!unionIterate_(in_buffer1, docBuffer1, blockDocid1, blockScore1, id0, c1, t1, i1, pointer1, id1, sc1))
                return;
        }
        else if (LESS_THAN(id0, id1, reverse_))
        {
            if (!unionIterate_(in_buffer0, docBuffer0, blockDocid0, blockScore0, id1, c0, t0, i0, pointer0, id0, sc0))
                return;
        }
        else
        {
            if (!unionIterate_(in_buffer1, docBuffer1, blockDocid1, blockScore1, id0, c1, t1, i1, pointer1, id1, sc1))
                return;
        }
    }
}

void AttrScoreInvertedIndex::intersectSetPostingsList_(
        uint32_t term,
        int weight,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    uint32_t iSet = 0, iCurrent = 0;

    size_t pointer = pointers_.headPointers.get(term);
    uint32_t blockDocid[BP_BLOCK_SIZE + 15] __attribute__((aligned(16)));
    uint32_t blockScore[BP_BLOCK_SIZE] __attribute__((aligned(16)));

    boost::shared_array<uint32_t> buffer(buffer_.getBuffer(term));
    const uint32_t* docBuffer = &buffer[4];

    uint32_t c = reverse_ ? buffer[0] : 0;
    uint32_t t = reverse_ ? buffer[c + 3] : 0;
    uint32_t i = reverse_ ? buffer[0] - buffer[1] : 0;
    uint32_t id = 0, sc = 0;
    uint32_t iCount = docid_list.size();
    uint32_t tail = docid_list.back();

    bool in_buffer = reverse_;

    if (!unionIterate_(in_buffer, docBuffer, blockDocid, blockScore, docid_list[iCurrent], c, t, i, pointer, id, sc))
        return;

    if (iCurrent >= iCount || LESS_THAN(tail, id, reverse_)
            || (iCurrent = linearSearch(&docid_list[0], reverse_, iCount, iCurrent, id)) == INVALID_ID)
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

        if (!unionIterate_(in_buffer, docBuffer, blockDocid, blockScore, docid_list[iCurrent], c, t, i, pointer, id, sc))
            break;

        if (iCurrent >= iCount || LESS_THAN(tail, id, reverse_)
                || (iCurrent = linearSearch(&docid_list[0], reverse_, iCount, iCurrent, id)) == INVALID_ID)
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
    if (qTerms.size() == 1)
    {
        uint32_t blockDocid[BP_BLOCK_SIZE + 15] __attribute__((aligned(16)));
        uint32_t blockScore[BP_BLOCK_SIZE] __attribute__((aligned(16)));
        uint32_t length = std::min(minDf, hits);

        docid_list.reserve(length + 15);
        score_list.reserve(length);

        boost::shared_array<uint32_t> buffer(buffer_.getBuffer(qTerms[0]));
        const uint32_t* docBuffer = &buffer[4];
        size_t pointer = pointers_.headPointers.get(qTerms[0]);
        uint32_t c = reverse_ ? buffer[0] : 0;
        uint32_t t = reverse_ ? buffer[c + 3] : 0;
        uint32_t i = reverse_ ? buffer[0] - buffer[1] : 0;
        uint32_t id = 0, sc = 0;

        bool in_buffer = reverse_;

        uint32_t eligible = filter->find_first(reverse_);
        if (eligible == INVALID_ID) return;

        if (!unionIterate_(in_buffer, docBuffer, blockDocid, blockScore, eligible, c, t, i, pointer, id, sc))
            return;

        if ((eligible = filter->test(id) ? id : filter->find_next(id, reverse_)) == INVALID_ID)
            return;

        while (true)
        {
            if (id == eligible)
            {
                docid_list.push_back(id);
                score_list.push_back(sc);

                if (docid_list.size() == length)
                    break;

                if ((eligible = filter->find_next(eligible, reverse_)) == INVALID_ID)
                    break;
            }

            if (!unionIterate_(in_buffer, docBuffer, blockDocid, blockScore, eligible, c, t, i, pointer, id, sc))
                break;

            if ((eligible = filter->test(id) ? id : filter->find_next(id, reverse_)) == INVALID_ID)
                break;
        }

        return;
    }

    docid_list.reserve(minDf + 15);
    score_list.reserve(minDf);

    intersectPostingsLists_(filter, qTerms[0], qTerms[1], qScores[0], qScores[1], docid_list, score_list, hits);

    for (uint32_t i = 2; i < qTerms.size(); ++i)
    {
        if (docid_list.empty()) return;
        intersectSetPostingsList_(qTerms[i], qScores[i], docid_list, score_list);
    }

    if (hits < docid_list.size())
    {
        docid_list.resize(hits);
        score_list.resize(hits);
    }
}

}

NS_IZENELIB_IR_END
