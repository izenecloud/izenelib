#include <ir/Zambezi/PositionalInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>

#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <glog/logging.h>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace
{

inline bool termCompare(const boost::tuple<uint32_t, uint32_t, size_t>& t1, const boost::tuple<uint32_t, uint32_t, size_t>& t2)
{
    return t1.get<0>() < t2.get<0>();
}

typedef std::pair<float, uint32_t> ScoreDocId;

inline bool compareDocIdLess(const ScoreDocId& x, const ScoreDocId& y)
{
    return x.second < y.second;
}

inline bool compareDocIdGreater(const ScoreDocId& x, const ScoreDocId& y)
{
    return x.second > y.second;
}

}

PositionalInvertedIndex::PositionalInvertedIndex(
        IndexType type,
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        uint32_t vocabSize,
        bool reverse,
        bool bloomEnabled,
        uint32_t nbHash,
        uint32_t bitsPerElement)
    : type_(type)
    , buffer_(vocabSize, type)
    , pool_(maxPoolSize, numberOfPools)
    , dictionary_(vocabSize)
    , pointers_(vocabSize, DEFAULT_COLLECTION_SIZE)
    , bloomEnabled_(bloomEnabled)
    , nbHash_(nbHash)
    , bitsPerElement_(bitsPerElement)
    , reverse_(reverse)
{
}

PositionalInvertedIndex::~PositionalInvertedIndex()
{
}

void PositionalInvertedIndex::save(std::ostream& ostr) const
{
    LOG(INFO) << "Save: start....";

    ostr.write((const char*)&reverse_, sizeof(reverse_));

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

    ostr.write((const char*)&bloomEnabled_, sizeof(bloomEnabled_));
    ostr.write((const char*)&nbHash_, sizeof(nbHash_));
    ostr.write((const char*)&bitsPerElement_, sizeof(bitsPerElement_));

    LOG(INFO) << "Save: done!";
}

void PositionalInvertedIndex::load(std::istream& istr)
{
    LOG(INFO) << "Load: start....";

    istr.read((char*)&reverse_, sizeof(reverse_));

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

    istr.read((char*)&bloomEnabled_, sizeof(bloomEnabled_));
    istr.read((char*)&nbHash_, sizeof(nbHash_));
    istr.read((char*)&bitsPerElement_, sizeof(bitsPerElement_));

    LOG(INFO) << "Load: done!";
}

uint32_t PositionalInvertedIndex::totalDocNum() const
{
    return pointers_.totalDocs;
}

void PositionalInvertedIndex::insertDoc(uint32_t docid,
                     const std::vector<std::string>& term_list,
                     const std::vector<uint32_t>& score_list)
{
    std::set<uint32_t> uniqueTerms;
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

        bool added = uniqueTerms.insert(id).second;

        pointers_.cf.increment(id);

        if (type_ == TF_ONLY)
        {
            std::vector<uint32_t>& tfBuffer = buffer_.tf[id];
            if (tfBuffer.capacity() == 0)
            {
                tfBuffer.reserve(DF_CUTOFF + 1);
            }
            if (added)
            {
                ++tfBuffer.back();
            }
            else
            {
                tfBuffer.push_back(1);
            }
        }
        else if (type_ == POSITIONAL)
        {
            std::vector<uint32_t>& tfBuffer = buffer_.tf[id];
            std::vector<uint32_t>& posBuffer = buffer_.position[id];

            if (posBuffer.capacity() == 0)
            {
                posBuffer.reserve(BLOCK_SIZE);
                tfBuffer.reserve(DF_CUTOFF + 1);
            }

            if (added)
            {
                posBuffer.push_back(i + 1);
                tfBuffer.push_back(1);
            }
            else
            {
                posBuffer.push_back(i + 1 - posBuffer.back());
                ++tfBuffer.back();
            }
        }
    }

    pointers_.docLen.set(docid, term_list.size());
    ++pointers_.totalDocs;

    for (std::set<uint32_t>::const_iterator it = uniqueTerms.begin();
            it != uniqueTerms.end(); ++it)
    {
        uint32_t id = *it;
        std::vector<uint32_t>& docBuffer = buffer_.docid[id];
        std::vector<uint32_t>& tfBuffer = buffer_.tf[id];
        std::vector<uint32_t>& posBuffer = buffer_.position[id];

        if (type_ != NON_POSITIONAL)
        {
            uint32_t tf = tfBuffer.back();
            uint32_t dl = pointers_.docLen.get(docid);
            float bm25TfScore = default_bm25tf(
                    tf, dl,
                    pointers_.totalDocLen /
                    (float)pointers_.totalDocs);
            float maxBm25TfScore = default_bm25tf(
                    pointers_.maxTf.get(id),
                    pointers_.maxTfDocLen.get(id),
                    pointers_.totalDocLen /
                    (float)pointers_.totalDocs);
            if (bm25TfScore > maxBm25TfScore)
            {
                pointers_.setMaxTf(id, tf, dl);
            }
        }

        pointers_.df.increment(id);
        if (pointers_.df.get(id) <= DF_CUTOFF)
        {
            if (docBuffer.capacity() == 0)
            {
                docBuffer.reserve(DF_CUTOFF);
            }
            docBuffer.push_back(docid);
            continue;
        }

        if (docBuffer.capacity() < BLOCK_SIZE)
        {
            docBuffer.reserve(BLOCK_SIZE);

            if (type_ == TF_ONLY || type_ == POSITIONAL)
            {
                tfBuffer.reserve(BLOCK_SIZE);
            }

            if (type_ == POSITIONAL)
            {
                uint32_t len = 2 * ((posBuffer.capacity() / BLOCK_SIZE) + 1) * BLOCK_SIZE;
                posBuffer.reserve(len);
            }
        }

        docBuffer.push_back(docid);

        if (type_ == POSITIONAL && docBuffer.size() % BLOCK_SIZE == 0)
        {
            buffer_.posBlockCount[id].push_back(posBuffer.size());
            posBuffer.resize((posBuffer.size() + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE);
        }

        if (docBuffer.size() == docBuffer.capacity())
        {
            processTermBuffer_(
                    buffer_.docid[id],
                    buffer_.tf[id],
                    buffer_.position[id],
                    buffer_.posBlockCount[id],
                    buffer_.tailPointer[id],
                    pointers_.headPointers.get(id));

            if (docBuffer.capacity() < MAX_BLOCK_SIZE)
            {
                uint32_t newLen = docBuffer.capacity() * EXPANSION_RATE;
                docBuffer.reserve(newLen);

                if (type_ != NON_POSITIONAL)
                {
                    tfBuffer.reserve(newLen);
                }
            }
        }
    }
}

void PositionalInvertedIndex::flush()
{
    uint32_t term = INVALID_ID;
    while ((term = buffer_.nextIndex(term, DF_CUTOFF)) != INVALID_ID)
    {
        processTermBuffer_(
                buffer_.docid[term],
                buffer_.tf[term],
                buffer_.position[term],
                buffer_.posBlockCount[term],
                buffer_.tailPointer[term],
                pointers_.headPointers.get(term));
    }
}

void PositionalInvertedIndex::processTermBuffer_(
        std::vector<uint32_t>& docBuffer,
        std::vector<uint32_t>& tfBuffer,
        std::vector<uint32_t>& posBuffer,
        std::vector<uint32_t>& posCountBuffer,
        size_t& tailPointer,
        size_t& headPointer)
{
    uint32_t nb = docBuffer.size() / BLOCK_SIZE;
    uint32_t res = docBuffer.size() % BLOCK_SIZE;

    if (reverse_)
    {
        size_t curPointer = UNDEFINED_POINTER;
        size_t lastPointer = UNDEFINED_POINTER;

        uint32_t ps = posCountBuffer.empty() ? 0 : posCountBuffer[nb - 1];

        if (res > 0)
        {
            uint32_t ceil_ps = (ps + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;

            lastPointer = compressAndAppendBlock_(
                    codec_,
                    &docBuffer[nb * BLOCK_SIZE],
                    &tfBuffer[nb * BLOCK_SIZE],
                    &posBuffer[ceil_ps],
                    res,
                    tfBuffer.empty() ? 0 : res,
                    posBuffer.size() - ceil_ps,
                    lastPointer,
                    tailPointer);


            if (curPointer == UNDEFINED_POINTER)
            {
                curPointer = lastPointer;
            }
        }

        for (int i = nb - 1; i >= 0; --i)
        {
            uint32_t last_ps = posCountBuffer.empty() || i == 0 ? 0 : posCountBuffer[i - 1];
            uint32_t ceil_ps = (last_ps + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;

            lastPointer = compressAndAppendBlock_(
                    codec_,
                    &docBuffer[i * BLOCK_SIZE],
                    &tfBuffer[i * BLOCK_SIZE],
                    &posBuffer[ceil_ps],
                    BLOCK_SIZE,
                    tfBuffer.empty() ? 0 : BLOCK_SIZE,
                    ps - ceil_ps,
                    lastPointer,
                    tailPointer);

            ps = last_ps;

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
        uint32_t ps = 0;

        for (uint32_t i = 0; i < nb; ++i)
        {
            uint32_t next_ps = posCountBuffer.empty() ? 0 : posCountBuffer[i];

            tailPointer = compressAndAppendBlock_(
                    codec_,
                    &docBuffer[i * BLOCK_SIZE],
                    &tfBuffer[i * BLOCK_SIZE],
                    &posBuffer[ps],
                    BLOCK_SIZE,
                    tfBuffer.empty() ? 0 : BLOCK_SIZE,
                    next_ps - ps,
                    tailPointer,
                    UNDEFINED_POINTER);

            ps = (next_ps + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;

            if (headPointer == UNDEFINED_POINTER)
            {
                headPointer = tailPointer;
            }
        }

        if (res > 0)
        {
            tailPointer = compressAndAppendBlock_(
                    codec_,
                    &docBuffer[nb * BLOCK_SIZE],
                    &tfBuffer[nb * BLOCK_SIZE],
                    &posBuffer[ps],
                    res,
                    tfBuffer.empty() ? 0 : res,
                    posBuffer.size() - ps,
                    tailPointer,
                    UNDEFINED_POINTER);

            if (headPointer == UNDEFINED_POINTER)
            {
                headPointer = tailPointer;
            }
        }
    }

    docBuffer.clear();
    tfBuffer.clear();
    posBuffer.clear();
    posCountBuffer.clear();
}

size_t PositionalInvertedIndex::compressAndAppendBlock_(
        FastPFor& codec,
        uint32_t* docBlock,
        uint32_t* tfBlock,
        uint32_t* posBlock,
        uint32_t len,
        uint32_t tflen,
        uint32_t plen,
        size_t lastPointer,
        size_t nextPointer)
{
    uint32_t maxDocId = reverse_ ? docBlock[0] : docBlock[len - 1];

    uint32_t filterSize = 0;
    if (bloomEnabled_)
    {
        filterSize = BloomFilter::computeLength(len, bitsPerElement_);
        memset(&segment_[BUFFER_SIZE - filterSize], 0, filterSize * sizeof(segment_[0]));
        for (uint32_t i = 0; i < len; ++i)
        {
            BloomFilter::insert(&segment_[BUFFER_SIZE - filterSize], filterSize, nbHash_, docBlock[i]);
        }
    }

    for (uint32_t i = len - 1; i > 0; --i)
    {
        docBlock[i] -= docBlock[i - 1];
    }

    if (reverse_)
    {
        std::reverse(docBlock, docBlock + len);
        std::reverse(tfBlock, tfBlock + tflen);

        std::vector<uint32_t> rpositions(plen);
        uint32_t curPos = plen, newPos = 0;
        for (uint32_t i = 0; i < tflen; ++i)
        {
            curPos -= tfBlock[i];
            memcpy(&rpositions[newPos], &posBlock[curPos], tfBlock[i] * sizeof(rpositions[0]));
            newPos += tfBlock[i];
        }
        memcpy(posBlock, &rpositions[0], plen * sizeof(posBlock[0]));
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docBlock[len], 0, (BLOCK_SIZE - len) * sizeof(docBlock[0]));
        if (tflen == len)
        {
            memset(&tfBlock[tflen], 0, (BLOCK_SIZE - tflen) * sizeof(tfBlock[0]));
        }
    }

    uint32_t reqspace = 0;
    segment_[reqspace++] = 0;
    segment_[reqspace++] = len;

    size_t csize = BLOCK_SIZE * 2;
    memset(&segment_[reqspace + 1], 0, csize * sizeof(segment_[0]));
    codec_.encodeArray(docBlock, BLOCK_SIZE, &segment_[reqspace + 1], csize);
    segment_[reqspace] = csize;
    reqspace += csize + 1;

    if (tflen > 0)
    {
        size_t tfcsize = BLOCK_SIZE * 2;
        memset(&segment_[reqspace + 1], 0, tfcsize * sizeof(segment_[0]));
        codec_.encodeArray(tfBlock, BLOCK_SIZE, &segment_[reqspace + 1], tfcsize);
        segment_[reqspace] = tfcsize;
        reqspace += tfcsize + 1;

        if (plen > 0)
        {
            segment_[reqspace++] = plen;

            uint32_t nb = plen / BLOCK_SIZE;
            uint32_t res = plen % BLOCK_SIZE;

            for (uint32_t i = 0; i < nb; ++i)
            {
                size_t tempPcsize = BLOCK_SIZE * 2;
                memset(&segment_[reqspace + 1], 0, tempPcsize * sizeof(segment_[0]));
                codec.encodeArray(&posBlock[i * BLOCK_SIZE], BLOCK_SIZE, &segment_[reqspace + 1], tempPcsize);
                segment_[reqspace] = tempPcsize;
                reqspace += tempPcsize + 1;
            }

            if (res > 0)
            {
                size_t tempPcsize = BLOCK_SIZE * 2;
                memset(&posBlock[plen], 0, (BLOCK_SIZE - res) * sizeof(posBlock[0]));
                memset(&segment_[reqspace + 1], 0, tempPcsize * sizeof(segment_[0]));
                codec.encodeArray(&posBlock[nb * BLOCK_SIZE], BLOCK_SIZE, &segment_[reqspace + 1], tempPcsize);
                segment_[reqspace] = tempPcsize;
                reqspace += tempPcsize + 1;
            }
        }
    }

    segment_[0] = reqspace + 4;

    if (bloomEnabled_)
    {
        segment_[reqspace] = filterSize;
        memcpy(&segment_[reqspace + 1], &segment_[BUFFER_SIZE - filterSize], filterSize * sizeof(segment_[0]));
        reqspace += filterSize + 1;
    }

    return pool_.appendSegment(segment_, maxDocId, reqspace, lastPointer, nextPointer);
}

void PositionalInvertedIndex::retrieve(
        Algorithm algorithm,
        const std::vector<std::pair<std::string, int> >& term_list_pair,
        const FilterBase* filter,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    std::vector<std::string> term_list;
    for (std::vector<std::pair<std::string, int> >::const_iterator i = term_list_pair.begin();
         i != term_list_pair.end(); ++i)
    {
        term_list.push_back(i->first);
    }

    std::vector<boost::tuple<uint32_t, uint32_t, size_t> > queries;
    uint32_t minimumDf = 0xFFFFFFFF;
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getTermId(term_list[i]);
        if (termid != INVALID_ID)
        {
            size_t pointer = pointers_.headPointers.get(termid);
            if (pointer != UNDEFINED_POINTER)
            {
                queries.push_back(boost::make_tuple(pointers_.df.get(termid), termid, pointer));
                minimumDf = std::min(queries.back().get<0>(), minimumDf);
            }
        }
    }

    if (queries.empty()) return;

    if (algorithm == BWAND_OR || algorithm == BWAND_AND || algorithm == SVS) // get the shortest posting
    {
        std::sort(queries.begin(), queries.end(), termCompare);
    }

    std::vector<uint32_t> qdf(queries.size());
    std::vector<size_t> qHeadPointers(queries.size());
    for (uint32_t i = 0; i < queries.size(); ++i)
    {
        qdf[i] = queries[i].get<0>();
        qHeadPointers[i] = queries[i].get<2>();
    }

    if (algorithm == BWAND_OR)
    {
        std::vector<float> UB(queries.size());
        for (uint32_t i = 0; i < queries.size(); ++i)
        {
            UB[i] = idf(pointers_.totalDocs, qdf[i]);
        }
        bwandOr_(qHeadPointers, UB, filter, hits, docid_list, score_list);
    }
    else if (algorithm == BWAND_AND)
    {
        if (!hits)
        {
            hits = minimumDf;
        }
        bwandAnd_(qHeadPointers, filter, hits, docid_list);
    }
    else if (algorithm == WAND || algorithm == MBWAND)
    {
        std::vector<float> UB(queries.size());
        if (algorithm == WAND && type_ != NON_POSITIONAL)
        {
            for (uint32_t i = 0; i < queries.size(); ++i)
            {
                UB[i] = default_bm25(
                        pointers_.maxTf.get(queries[i].get<1>()),
                        qdf[i],
                        pointers_.totalDocs,
                        pointers_.maxTfDocLen.get(queries[i].get<1>()),
                        pointers_.totalDocLen / (float)pointers_.totalDocs);
            }
        }
        else
        {
            for (uint32_t i = 0; i < queries.size(); ++i)
            {
                UB[i] = idf(pointers_.totalDocs, qdf[i]);
            }
        }

        wand_(
                qHeadPointers,
                qdf,
                UB,
                pointers_.docLen.getCounter(),
                filter,
                pointers_.totalDocs,
                pointers_.totalDocLen / (float)pointers_.totalDocs,
                hits,
                algorithm == WAND && type_ != NON_POSITIONAL,
                docid_list,
                score_list);
    }
    else if (algorithm == SVS)
    {
        intersectSvS_(qHeadPointers, filter, minimumDf, hits, docid_list);
    }

    score_list.resize(docid_list.size());
}

uint32_t PositionalInvertedIndex::decompressDocidBlock_(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    const uint32_t* block = &pool_.pool_[pSegment][pOffset + 7];
    size_t csize = pool_.pool_[pSegment][pOffset + 6];
    size_t nvalue = BLOCK_SIZE;
    memset(outBlock, 0, BLOCK_SIZE * sizeof(outBlock[0]));
    codec.decodeArray(block, csize, outBlock, nvalue);

    uint32_t len = pool_.pool_[pSegment][pOffset + 5];

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

uint32_t PositionalInvertedIndex::decompressTfBlock_(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    const uint32_t* block = &pool_.pool_[pSegment][pOffset + csize + 8];
    size_t tfcsize = pool_.pool_[pSegment][pOffset + csize + 7];
    size_t nvalue = BLOCK_SIZE;
    memset(outBlock, 0, BLOCK_SIZE * sizeof(outBlock[0]));
    codec.decodeArray(block, tfcsize, outBlock, nvalue);

    return pool_.pool_[pSegment][pOffset + 5];
}

uint32_t PositionalInvertedIndex::numberOfPositionBlocks_(size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_.pool_[pSegment][pOffset + csize + 7];
    uint32_t plen = pool_.pool_[pSegment][pOffset + csize + tfcsize + 8];

    return (plen + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

uint32_t PositionalInvertedIndex::decompressPositionBlock_(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_.pool_[pSegment][pOffset + csize + 7];
    uint32_t plen = pool_.pool_[pSegment][pOffset + csize + tfcsize + 8];
    uint32_t nb = (plen + BLOCK_SIZE - 1) / BLOCK_SIZE;

    uint32_t index = pOffset + csize + tfcsize + 9;
    for (uint32_t i = 0; i < nb; ++i)
    {
        uint32_t sb = pool_.pool_[pSegment][index];
        size_t nvalue = BLOCK_SIZE;
        codec.decodeArray(&pool_.pool_[pSegment][index + 1], sb, &outBlock[i * BLOCK_SIZE], nvalue);
        index += sb + 1;
    }
    return plen;
}

void PositionalInvertedIndex::decompressPositions_(
        FastPFor& codec,
        uint32_t* tf_list, uint32_t index, size_t pointer, uint32_t* out) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_.pool_[pSegment][pOffset + 7 + csize];
    uint32_t nb = 0;
    for (uint32_t i = 0; i < index; ++i)
    {
        nb += tf_list[i];
    }
    uint32_t lnb = nb + tf_list[index] - 1;
    uint32_t r = nb % BLOCK_SIZE;
    nb = nb / BLOCK_SIZE;
    lnb = lnb / BLOCK_SIZE;

    uint32_t pos = pOffset + csize + tfcsize + 9;
    for (uint32_t i = 0; i < nb; ++i)
    {
        pos += pool_.pool_[pSegment][pos] + 1;
    }
    uint32_t cindex = 0, left = tf_list[index], tocopy = tf_list[index], rindex = r;
    for (uint32_t i = nb; i <= lnb; ++i)
    {
        if (rindex + tocopy > BLOCK_SIZE)
        {
            tocopy = BLOCK_SIZE - rindex;
        }
        uint32_t sb = pool_.pool_[pSegment][pos];
        std::vector<uint32_t> temp(BLOCK_SIZE);
        size_t nvalue = BLOCK_SIZE;
        codec.decodeArray(&pool_.pool_[pSegment][pos + 1], sb, &temp[0], nvalue);
        memcpy(&out[cindex], &temp[rindex], tocopy * sizeof(out[0]));
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

bool PositionalInvertedIndex::containsDocid_(uint32_t docid, size_t& pointer) const
{
    if (pointer == UNDEFINED_POINTER)
    {
        return false;
    }
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    while (LESS_THAN(pool_.pool_[pSegment][pOffset + 3], docid, reverse_))
    {
        uint32_t oldSegment = pSegment;
        uint32_t oldOffset = pOffset;
        pSegment = pool_.pool_[oldSegment][oldOffset + 1];
        if (pSegment == UNDEFINED_SEGMENT)
        {
            pointer = UNDEFINED_POINTER;
            return false;
        }
        pOffset = pool_.pool_[oldSegment][oldOffset + 2];
    }

    if (pool_.pool_[pSegment][pOffset + 3] == docid)
    {
        return true;
    }

    uint32_t bloomOffset = pool_.pool_[pSegment][pOffset + 4];

    pointer = ENCODE_POINTER(pSegment, pOffset);
    return BloomFilter::contains(
            &pool_.pool_[pSegment][pOffset + bloomOffset + 1],
            pool_.pool_[pSegment][pOffset + bloomOffset],
            nbHash_, docid);
}

void PositionalInvertedIndex::bwandAnd_(
        std::vector<size_t>& headPointers,
        const FilterBase* filter,
        uint32_t hits,
        std::vector<uint32_t>& docid_list) const
{
    docid_list.reserve(hits);
    FastPFor codec;
    uint32_t block[BLOCK_SIZE];

    uint32_t c = 0, i = 0;

    uint32_t eligible = filter->find_first(reverse_);

    if (!iterateSegment_(codec, block, c, i, headPointers[0], eligible))
        return;

    if ((eligible = filter->test(block[i]) ? block[i] : filter->find_next(block[i], reverse_)) == INVALID_ID)
        return;

    while (true)
    {
        if (block[i] == eligible)
        {
            bool found = true;
            for (uint32_t j = 1; j < headPointers.size(); ++j)
            {
                if (!containsDocid_(eligible, headPointers[j]))
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                docid_list.push_back(eligible);
                if (docid_list.size() >= hits)
                    return;
            }

            if ((eligible = filter->find_next(eligible, reverse_)) == INVALID_ID)
                break;
        }

        if (!iterateSegment_(codec, block, c, i, headPointers[0], eligible))
            break;

        if ((eligible = filter->test(block[i]) ? block[i] : filter->find_next(block[i], reverse_)) == INVALID_ID)
            break;
    }
}

void PositionalInvertedIndex::bwandOr_(
        std::vector<size_t>& headPointers,
        const std::vector<float>& UB,
        const FilterBase* filter,
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
    uint32_t block[BLOCK_SIZE];

    uint32_t c = 0, i = 0;

    uint32_t eligible = filter->find_first(reverse_);

    if (!iterateSegment_(codec, block, c, i, headPointers[0], eligible))
        return;

    if ((eligible = filter->test(block[i]) ? block[i] : filter->find_next(block[i], reverse_)) == INVALID_ID)
        return;

    while (true)
    {
        if (block[i] == eligible)
        {
            float score = UB[0];
            for (uint32_t j = 1; j < headPointers.size(); ++j)
            {
                if (containsDocid_(eligible, headPointers[j]))
                {
                    score += UB[j];
                }
            }

            if (result_list.size() < hits)
            {
                result_list.push_back(std::make_pair(score, eligible));
                std::push_heap(result_list.begin(), result_list.end(), comparator);
            }
            else if (score > threshold)
            {
                std::pop_heap(result_list.begin(), result_list.end(), comparator);
                result_list.back() = std::make_pair(score, eligible);
                std::push_heap(result_list.begin(), result_list.end(), comparator);
            }

            threshold = result_list[0].first;
            if (result_list.size() == hits && threshold == sumOfUB)
                break;

            if ((eligible = filter->find_next(eligible, reverse_)) == INVALID_ID)
                break;
        }

        if (!iterateSegment_(codec, block, c, i, headPointers[0], eligible))
            break;

        if ((eligible = filter->test(block[i]) ? block[i] : filter->find_next(block[i], reverse_)) == INVALID_ID)
            break;
    }

    boost::function<bool (const ScoreDocId&, const ScoreDocId&)> docIdComparator =
            reverse_ ? compareDocIdGreater : compareDocIdLess;

    std::sort(result_list.begin(), result_list.end(), docIdComparator);

    for (uint32_t i = 0; i < result_list.size(); ++i)
    {
        score_list.push_back(result_list[i].first);
        docid_list.push_back(result_list[i].second);
    }
}

bool PositionalInvertedIndex::iterateSegment_(
        FastPFor& codec,
        uint32_t* block,
        uint32_t& count, uint32_t& index, size_t& pointer,
        uint32_t pivot) const
{
    if (count == 0 || LESS_THAN(block[count - 1], pivot, reverse_))
    {
        if ((pointer = pool_.nextPointer(pointer, pivot, reverse_)) == UNDEFINED_POINTER)
            return false;

        count = decompressDocidBlock_(codec, &block[0], pointer);
        index = 0;
    }

    return gallopSearch_(block, count, index, pivot);
}

bool PositionalInvertedIndex::gallopSearch_(
        const uint32_t* block,
        uint32_t count,
        uint32_t& index,
        uint32_t pivot) const
{
    if (index >= count || LESS_THAN(block[count - 1], pivot, reverse_))
        return false;

    if (GREATER_THAN_EQUAL(block[index], pivot, reverse_))
        return true;

    if (block[count - 1] == pivot)
    {
        index = count - 1;
        return true;
    }

    int beginIndex = index;
    int hop = 1;
    int tempIndex = beginIndex + 1;
    while ((uint32_t)tempIndex < count && LESS_THAN_EQUAL(block[tempIndex], pivot, reverse_))
    {
        beginIndex = tempIndex;
        tempIndex += hop;
        hop *= 2;
    }
    if (block[beginIndex] == pivot)
    {
        index = beginIndex;
        return true;
    }

    int endIndex = count - 1;
    hop = 1;
    tempIndex = endIndex - 1;
    while (tempIndex >= 0 && GREATER_THAN(block[tempIndex], pivot, reverse_))
    {
        endIndex = tempIndex;
        tempIndex -= hop;
        hop *= 2;
    }
    if (block[endIndex] == pivot)
    {
        index = endIndex;
        return true;
    }

    // Binary search between begin and end indexes
    while (beginIndex < endIndex)
    {
        uint32_t mid = beginIndex + (endIndex - beginIndex) / 2;

        if (LESS_THAN(pivot, block[mid], reverse_))
        {
            endIndex = mid;
        }
        else if (GREATER_THAN(pivot, block[mid], reverse_))
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

void PositionalInvertedIndex::wand_(
        std::vector<size_t>& headPointers,
        const std::vector<uint32_t>& df,
        const std::vector<float>& UB,
        const std::vector<uint32_t>& docLen,
        const FilterBase* filter,
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
    std::vector<uint32_t> mapping;
    float threshold = .0f;

    FastPFor codec;

    uint32_t eligible = filter->find_first(reverse_);

    if (eligible == INVALID_ID) return;

    mapping.reserve(len);

    for (uint32_t i = 0; i < len; ++i)
    {
        blockDocid[i].resize(BLOCK_SIZE);

        if (!iterateSegment_(codec, &blockDocid[i][0], counts[i], posting[i], headPointers[i], eligible))
            continue;

        if (hasTf)
        {
            blockTf[i].resize(BLOCK_SIZE);
            decompressTfBlock_(codec, &blockTf[i][0], headPointers[i]);
        }

        mapping.push_back(i);

        if (UB[i] <= threshold)
        {
            threshold = UB[i] - 1;
        }
    }

    if (mapping.empty()) return;
    len = mapping.size();

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
        uint32_t pTermIdx = INVALID_ID;
        for (uint32_t i = 0; i < len; ++i)
        {
            if ((sum += UB[mapping[i]]) > threshold && (i == len - 1 || blockDocid[mapping[i]][posting[mapping[i]]] != blockDocid[mapping[i + 1]][posting[mapping[i + 1]]]))
            {
                pTermIdx = i;
                break;
            }
        }

        if (sum == 0 || pTermIdx == INVALID_ID) break;

        uint32_t pTerm = mapping[pTermIdx];
        uint32_t pivot = blockDocid[pTerm][posting[pTerm]];

        if (blockDocid[mapping[0]][posting[mapping[0]]] == pivot && filter->test(pivot))
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
                }
                else if (score > result_list[0].first)
                {
                    std::pop_heap(result_list.begin(), result_list.end(), comparator);
                    result_list.back() = std::make_pair(score, pivot);
                    std::push_heap(result_list.begin(), result_list.end(), comparator);
                }

                if (result_list.size() == hits)
                {
                    if (!hasTf && len == 1) break;
                    threshold = result_list[0].first;
                }
            }
        }

        eligible = (blockDocid[mapping[0]][posting[mapping[0]]] != pivot && filter->test(pivot)) ? pivot : filter->find_next(pivot, reverse_);

        if (eligible == INVALID_ID) break;

        for (uint32_t i = 0; i < mapping.size(); ++i)
        {
            uint32_t aterm = mapping[i];
            if (blockDocid[aterm][posting[aterm]] >= eligible)
                break;

            size_t pointer = headPointers[aterm];
            if (!iterateSegment_(codec, &blockDocid[aterm][0], counts[aterm], posting[aterm], headPointers[aterm], eligible))
            {
                mapping[i] = INVALID_ID;
                continue;
            }

            if (hasTf && pointer != headPointers[aterm])
            {
                decompressTfBlock_(codec, &blockTf[aterm][0], headPointers[aterm]);
            }
        }

        mapping.erase(std::remove(mapping.begin(), mapping.end(), INVALID_ID), mapping.end());
        len = mapping.size();

        for (uint32_t i = 0; i < len; ++i)
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

void PositionalInvertedIndex::intersectPostingsLists_(
        FastPFor& codec,
        const FilterBase* filter,
        size_t pointer0, size_t pointer1,
        std::vector<uint32_t>& docid_list) const
{
    uint32_t data0[BLOCK_SIZE];
    uint32_t data1[BLOCK_SIZE];

    uint32_t c0 = 0, c1 = 0;
    uint32_t i0 = 0, i1 = 0;

    uint32_t eligible = filter->find_first(reverse_);

    if (!iterateSegment_(codec, data0, c0, i0, pointer0, eligible))
        return;

    if ((eligible = filter->test(data0[i0]) ? data0[i0] : filter->find_next(data0[i0], reverse_)) == INVALID_ID)
        return;

    if (!iterateSegment_(codec, data1, c1, i1, pointer1, eligible))
        return;

    while (true)
    {
        if (data1[i1] == data0[i0])
        {
            docid_list.push_back(data0[i0]);

            if ((eligible = filter->find_next(data0[i0], reverse_)) == INVALID_ID)
                break;

            if (!iterateSegment_(codec, data0, c0, i0, pointer0, eligible))
                break;

            if ((eligible = filter->test(data0[i0]) ? data0[i0] : filter->find_next(data0[i0], reverse_)) == INVALID_ID)
                break;

            if (!iterateSegment_(codec, data1, c1, i1, pointer1, eligible))
                break;
        }
        else if (LESS_THAN(data0[i0], data1[i1], reverse_))
        {
            if ((eligible = filter->test(data1[i1]) ? data1[i1] : filter->find_next(data1[i1], reverse_)) == INVALID_ID)
                break;

            if (!iterateSegment_(codec, data0, c0, i0, pointer0, eligible))
                break;
        }
        else
        {
            if ((eligible = filter->test(data0[i0]) ? data0[i0] : filter->find_next(data0[i0], reverse_)) == INVALID_ID)
                break;

            if (!iterateSegment_(codec, data1, c1, i1, pointer1, eligible))
                break;
        }
    }
}

void PositionalInvertedIndex::intersectSetPostingsList_(
        FastPFor& codec,
        size_t pointer,
        std::vector<uint32_t>& docid_list) const
{
    uint32_t block[BLOCK_SIZE];
    uint32_t c = 0, i = 0;
    uint32_t iSet = 0, iCurrent = 0;

    if (!iterateSegment_(codec, block, c, i, pointer, docid_list[iCurrent]))
        return;

    if (!gallopSearch_(&docid_list[0], docid_list.size(), iCurrent, block[i]))
        return;

    while (true)
    {
        if (block[i] == docid_list[iCurrent])
        {
            docid_list[iSet++] = docid_list[iCurrent++];

            if (iCurrent == docid_list.size())
                break;
        }

        if (!iterateSegment_(codec, block, c, i, pointer, docid_list[iCurrent]))
            break;

        if (!gallopSearch_(&docid_list[0], docid_list.size(), iCurrent, block[i]))
            break;
    }

    docid_list.resize(iSet);
}

void PositionalInvertedIndex::intersectSvS_(
        std::vector<size_t>& headPointers,
        const FilterBase* filter,
        uint32_t minDf,
        uint32_t hits,
        std::vector<uint32_t>& docid_list) const
{
    FastPFor codec;

    if (headPointers.size() == 1)
    {
        uint32_t block[BLOCK_SIZE];
        uint32_t length = std::min(minDf, hits);

        docid_list.reserve(length);

        uint32_t c = 0, i = 0;

        uint32_t eligible = filter->find_first(reverse_);

        if (!iterateSegment_(codec, block, c, i, headPointers[0], eligible))
            return;

        if ((eligible = filter->test(block[i]) ? block[i] : filter->find_next(block[i], reverse_)) == INVALID_ID)
            return;

        while (true)
        {
            if (block[i] == eligible)
            {
                docid_list.push_back(eligible);

                if (docid_list.size() == length)
                    break;

                if ((eligible = filter->find_next(eligible, reverse_)) == INVALID_ID)
                    break;
            }

            if (!iterateSegment_(codec, block, c, i, headPointers[0], eligible))
                break;

            if ((eligible = filter->test(block[i]) ? block[i] : filter->find_next(block[i], reverse_)) == INVALID_ID)
                break;
        }

        return;
    }

    docid_list.reserve(minDf);
    intersectPostingsLists_(codec, filter, headPointers[0], headPointers[1], docid_list);
    for (uint32_t i = 2; i < headPointers.size(); ++i)
    {
        if (docid_list.empty()) return;
        intersectSetPostingsList_(codec, headPointers[i], docid_list);
    }

    if (hits < docid_list.size())
        docid_list.resize(hits);
}

}

NS_IZENELIB_IR_END
