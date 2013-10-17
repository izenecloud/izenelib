#include <ir/Zambezi/PositionalInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>
#include <ir/Zambezi/bloom/BloomFilter.hpp>

#include <boost/tuple/tuple.hpp>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace
{

inline bool termCompare(const boost::tuple<uint32_t, uint32_t, size_t>& t1, const boost::tuple<uint32_t, uint32_t, size_t>& t2)
{
    return t1.get<0>() < t2.get<0>();
}

}

PositionalInvertedIndex::PositionalInvertedIndex(
        IndexType type,
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        bool reverse,
        bool bloomEnabled,
        uint32_t nbHash,
        uint32_t bitsPerElement)
    : type_(type)
    , buffer_(DEFAULT_VOCAB_SIZE, type)
    , pool_(maxPoolSize, numberOfPools, reverse)
    , dictionary_(DEFAULT_VOCAB_SIZE)
    , pointers_(DEFAULT_VOCAB_SIZE, DEFAULT_COLLECTION_SIZE)
    , bloomEnabled_(bloomEnabled)
    , nbHash_(nbHash)
    , bitsPerElement_(bitsPerElement)
{
}

PositionalInvertedIndex::~PositionalInvertedIndex()
{
}

void PositionalInvertedIndex::save(std::ostream& ostr) const
{
    buffer_.save(ostr);
    pool_.save(ostr);
    dictionary_.save(ostr);
    pointers_.save(ostr);

    ostr.write((const char*)&bloomEnabled_, sizeof(bloomEnabled_));
    ostr.write((const char*)&nbHash_, sizeof(nbHash_));
    ostr.write((const char*)&bitsPerElement_, sizeof(bitsPerElement_));
}

void PositionalInvertedIndex::load(std::istream& istr)
{
    buffer_.load(istr);
    pool_.load(istr);
    dictionary_.load(istr);
    pointers_.load(istr);

    istr.read((char*)&bloomEnabled_, sizeof(bloomEnabled_));
    istr.read((char*)&nbHash_, sizeof(nbHash_));
    istr.read((char*)&bitsPerElement_, sizeof(bitsPerElement_));
}

void PositionalInvertedIndex::insertDoc(uint32_t docid, const std::vector<std::string>& term_list)
{
    std::set<uint32_t> uniqueTerms;
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t id = dictionary_.insertTerm(term_list[i]);
        bool added = uniqueTerms.insert(id).second;

        pointers_.cf_.increment(id);

        if (type_ == TF_ONLY)
        {
            std::vector<uint32_t>& tfBuffer = buffer_.tf_[id];
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
            std::vector<uint32_t>& tfBuffer = buffer_.tf_[id];
            std::vector<uint32_t>& posBuffer = buffer_.position_[id];

            if (posBuffer.capacity() == 0)
            {
                posBuffer.reserve(DF_CUTOFF);
                posBuffer.push_back(0);

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

            ++posBuffer[buffer_.posBlockHead_[id]];
        }
    }

    pointers_.docLen_.set(docid, term_list.size());

    for (std::set<uint32_t>::const_iterator it = uniqueTerms.begin();
            it != uniqueTerms.end(); ++it)
    {
        uint32_t id = *it;
        std::vector<uint32_t>& docBuffer = buffer_.docid_[id];
        std::vector<uint32_t>& tfBuffer = buffer_.tf_[id];
        std::vector<uint32_t>& posBuffer = buffer_.position_[id];

        if (type_ != NON_POSITIONAL)
        {
            uint32_t tf = tfBuffer.back();
            uint32_t dl = pointers_.docLen_.get(docid);
            float bm25TfScore = default_bm25tf(
                    tf, dl,
                    pointers_.totalDocLen_ /
                    (float)pointers_.totalDocs_);
            float maxBm25TfScore = default_bm25tf(
                    pointers_.maxTf_.get(id),
                    pointers_.maxTfDocLen_.get(id),
                    pointers_.totalDocLen_ /
                    (float)pointers_.totalDocs_);
            if (bm25TfScore > maxBm25TfScore)
            {
                pointers_.setMaxTf(id, tf, dl);
            }
        }

        uint32_t df = pointers_.df_.get(id);
        if (df < DF_CUTOFF)
        {
            if (docBuffer.capacity() == 0)
            {
                docBuffer.reserve(DF_CUTOFF);
            }
            docBuffer.push_back(docid);
            pointers_.df_.increment(id);
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
        pointers_.df_.increment(id);

        if (type_ == POSITIONAL && docBuffer.size() % BLOCK_SIZE == 0)
        {
            buffer_.posBlockHead_[id] = posBuffer.size();
            posBuffer.push_back(0);
        }

        if (docBuffer.size() == docBuffer.capacity())
        {
            uint32_t nb = docBuffer.size() / BLOCK_SIZE;
            size_t pointer = buffer_.tailPointer_[id];

            for (uint32_t j = 0, ps = 0; j < nb; ++j)
            {
                switch (type_)
                {
                    case NON_POSITIONAL:
                        pointer = compressAndAddNonPositional_(
                                codec_,
                                &docBuffer[j * BLOCK_SIZE],
                                BLOCK_SIZE,
                                pointer);
                        break;

                    case TF_ONLY:
                        pointer = compressAndAddTfOnly_(
                                codec_,
                                &docBuffer[j * BLOCK_SIZE],
                                &tfBuffer[j * BLOCK_SIZE],
                                BLOCK_SIZE,
                                pointer);
                        break;

                    case POSITIONAL:
                        pointer = compressAndAddPositional_(
                                codec_,
                                &docBuffer[j * BLOCK_SIZE],
                                &tfBuffer[j * BLOCK_SIZE],
                                &posBuffer[ps + 1],
                                BLOCK_SIZE,
                                posBuffer[ps],
                                pointer);
                        ps += posBuffer[ps] + 1;
                        break;

                    default:
                        break;
                }

                if (pool_.reverse_ || pointers_.headPointers_.get(id) == UNDEFINED_POINTER)
                {
                    pointers_.headPointers_.set(id, pointer);
                }
            }

            buffer_.tailPointer_[id] = pointer;

            docBuffer.clear();
            if (type_ != NON_POSITIONAL)
            {
                tfBuffer.clear();
            }

            if (docBuffer.capacity() < MAX_BLOCK_SIZE)
            {
                uint32_t newLen = docBuffer.capacity() * EXPANSION_RATE;
                docBuffer.reserve(newLen);

                if (type_ != NON_POSITIONAL)
                {
                    tfBuffer.reserve(newLen);
                }
            }

            if (type_ == POSITIONAL)
            {
                posBuffer.clear();
                posBuffer.push_back(0);
                buffer_.posBlockHead_[id] = 0;
            }
        }
    }
}

void PositionalInvertedIndex::flush()
{
    uint32_t term = -1;
    while ((term = buffer_.nextIndex(term, DF_CUTOFF)) != (uint32_t)-1)
    {
        std::vector<uint32_t>& docBuffer = buffer_.docid_[term];
        std::vector<uint32_t>& tfBuffer = buffer_.tf_[term];
        std::vector<uint32_t>& posBuffer = buffer_.position_[term];

        uint32_t pos = docBuffer.size();
        size_t pointer = buffer_.tailPointer_[term];

        uint32_t nb = pos / BLOCK_SIZE;
        uint32_t res = pos % BLOCK_SIZE;
        uint32_t ps = 0;

        for (uint32_t i = 0; i < nb; ++i)
        {
            switch (type_)
            {
            case NON_POSITIONAL:
                pointer = compressAndAddNonPositional_(
                        codec_,
                        &docBuffer[i * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);
                break;

            case TF_ONLY:
                pointer = compressAndAddTfOnly_(
                        codec_,
                        &docBuffer[i * BLOCK_SIZE],
                        &tfBuffer[i * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);
                break;

            case POSITIONAL:
                pointer = compressAndAddPositional_(
                        codec_,
                        &docBuffer[i * BLOCK_SIZE],
                        &tfBuffer[i * BLOCK_SIZE],
                        &posBuffer[ps + 1],
                        BLOCK_SIZE,
                        posBuffer[ps],
                        pointer);
                ps += posBuffer[ps] + 1;
                break;

            default:
                break;
            }

            if (pool_.reverse_ || pointers_.headPointers_.get(term) == UNDEFINED_POINTER)
            {
                pointers_.headPointers_.set(term, pointer);
            }
        }

        if (res > 0)
        {
            switch (type_)
            {
            case NON_POSITIONAL:
                pointer = compressAndAddNonPositional_(
                        codec_,
                        &docBuffer[nb * BLOCK_SIZE],
                        res,
                        pointer);
                break;

            case TF_ONLY:
                pointer = compressAndAddTfOnly_(
                        codec_,
                        &docBuffer[nb * BLOCK_SIZE],
                        &tfBuffer[nb * BLOCK_SIZE],
                        res,
                        pointer);
                break;

            case POSITIONAL:
                pointer = compressAndAddPositional_(
                        codec_,
                        &docBuffer[nb * BLOCK_SIZE],
                        &tfBuffer[nb * BLOCK_SIZE],
                        &posBuffer[ps + 1],
                        res,
                        posBuffer[ps],
                        pointer);
                break;

            default:
                break;
            }

            if (pool_.reverse_ || pointers_.headPointers_.get(term) == UNDEFINED_POINTER)
            {
                pointers_.headPointers_.set(term, pointer);
            }
        }

        buffer_.tailPointer_[term] = pointer;

        docBuffer.clear();

        if (type_ != NON_POSITIONAL)
            tfBuffer.clear();

        if (type_ == POSITIONAL)
            posBuffer.clear();
    }
}

size_t PositionalInvertedIndex::compressAndAddNonPositional_(
        FastPFor& codec,
        uint32_t* docid_list,
        uint32_t len, size_t tailPointer)
{
    uint32_t maxDocId = pool_.reverse_ ? docid_list[0] : docid_list[len - 1];

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

    if (pool_.reverse_)
    {
        std::reverse(docid_list, docid_list + len);
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);

    segment_[0] = csize + 7;
    segment_[1] = len;
    segment_[2] = csize;
    memcpy(&segment_[3], &block[0], csize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        segment_[csize + 3] = filterSize;
        memcpy(&segment_[csize + 4], &filter[0], filterSize * sizeof(uint32_t));
    }

    return pool_.appendSegment(segment_, maxDocId, csize + filterSize + 4, tailPointer);
}

size_t PositionalInvertedIndex::compressAndAddTfOnly_(
        FastPFor& codec,
        uint32_t* docid_list, uint32_t* tf_list,
        uint32_t len, size_t tailPointer)
{
    uint32_t maxDocId = pool_.reverse_ ? docid_list[0] : docid_list[len - 1];

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

    if (pool_.reverse_)
    {
        std::reverse(docid_list, docid_list + len);
        std::reverse(tf_list, tf_list + len);
    }

    if (len < BLOCK_SIZE)
    {
        memset(&docid_list[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
        memset(&tf_list[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
    }

    std::vector<uint32_t> block(BLOCK_SIZE * 2);
    std::vector<uint32_t> tfblock(BLOCK_SIZE * 2);
    size_t csize = BLOCK_SIZE * 2;
    codec.encodeArray(docid_list, BLOCK_SIZE, &block[0], csize);
    size_t tfcsize = BLOCK_SIZE * 2;
    codec.encodeArray(tf_list, BLOCK_SIZE, &tfblock[0], tfcsize);

    segment_[0] = csize + tfcsize + 8;
    segment_[1] = len;

    segment_[2] = csize;
    memcpy(&segment_[3], &block[0], csize * sizeof(uint32_t));

    segment_[csize + 3] = tfcsize;
    memcpy(&segment_[csize + 4], &tfblock[0], tfcsize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        segment_[csize + tfcsize + 4] = filterSize;
        memcpy(&segment_[csize + tfcsize + 5], &filter[0], filterSize * sizeof(uint32_t));
    }

    return pool_.appendSegment(segment_, maxDocId, csize + tfcsize + filterSize + 5, tailPointer);
}

size_t PositionalInvertedIndex::compressAndAddPositional_(
        FastPFor& codec,
        uint32_t* docid_list, uint32_t* tf_list, uint32_t* position_list,
        uint32_t len, uint32_t plen, size_t tailPointer)
{
    uint32_t maxDocId = pool_.reverse_ ? docid_list[0] : docid_list[len - 1];

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

    if (pool_.reverse_)
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
        memset(&docid_list[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
        memset(&tf_list[len], 0, (BLOCK_SIZE - len) * sizeof(uint32_t));
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
        memset(&position_list[plen], 0, (BLOCK_SIZE - res) * sizeof(uint32_t));
        size_t tempPcsize = BLOCK_SIZE * 2;
        codec.encodeArray(&position_list[nb * BLOCK_SIZE], BLOCK_SIZE, &pblock[pcsize + 1], tempPcsize);
        pblock[pcsize] = tempPcsize;
        pcsize += tempPcsize + 1;
    }

    segment_[0] = csize + tfcsize + pcsize + 10;
    segment_[1] = len;

    segment_[2] = csize;
    memcpy(&segment_[3], &block[0], csize * sizeof(uint32_t));

    segment_[csize + 3] = tfcsize;
    memcpy(&segment_[csize + 4], &tfblock[0], tfcsize * sizeof(uint32_t));

    segment_[csize + tfcsize + 4] = plen;
    segment_[csize + tfcsize + 5] = nb + (res ? 1 : 0);
    memcpy(&segment_[csize + tfcsize + 6], &pblock[0], pcsize * sizeof(uint32_t));

    if (bloomEnabled_)
    {
        segment_[csize + tfcsize + pcsize + 6] = filterSize;
        memcpy(&segment_[csize + tfcsize + pcsize + 7], &filter[0], filterSize * sizeof(uint32_t));
    }

    return pool_.appendSegment(segment_, maxDocId, csize + tfcsize + pcsize + filterSize + 7, tailPointer);
}

void PositionalInvertedIndex::retrieval(
        Algorithm algorithm,
        const std::vector<std::string>& term_list,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    std::vector<boost::tuple<uint32_t, uint32_t, size_t> > queries;
    uint32_t minimumDf = 0xFFFFFFFF;
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getTermId(term_list[i]);
        if (termid != INVALID_ID)
        {
            size_t pointer = pointers_.headPointers_.get(termid);
            if (pointer != UNDEFINED_POINTER)
            {
                queries.push_back(boost::make_tuple(pointers_.df_.get(termid), termid, pointer));
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
            UB[i] = idf(pointers_.totalDocs_, qdf[i]);
        }
        bwandOr_(qHeadPointers, UB, hits, docid_list, score_list);
    }
    else if (algorithm == BWAND_AND)
    {
        if (!hits)
        {
            hits = minimumDf;
        }
        bwandAnd_(qHeadPointers, hits, docid_list);
    }
    else if (algorithm == WAND || algorithm == MBWAND)
    {
        std::vector<float> UB(queries.size());
        for (uint32_t i = 0; i < queries.size(); ++i)
        {
            if (algorithm == WAND)
            {
                UB[i] = default_bm25(
                        pointers_.maxTf_.get(queries[i].get<1>()),
                        qdf[i],
                        pointers_.totalDocs_,
                        pointers_.maxTfDocLen_.get(queries[i].get<1>()),
                        pointers_.totalDocLen_ / (float)pointers_.totalDocs_);
            }
            else
            {
                UB[i] = idf(pointers_.totalDocs_, qdf[i]);
            }
        }
        wand_(
                qHeadPointers,
                qdf,
                UB,
                pointers_.docLen_.getCounter(),
                pointers_.totalDocs_,
                pointers_.totalDocLen_ / (float)pointers_.totalDocs_,
                hits,
                algorithm == WAND,
                docid_list,
                score_list);
    }
    else if (algorithm == SVS)
    {
        intersectSvS_(qHeadPointers, minimumDf, hits, docid_list);
    }
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
    memset(outBlock, 0, BLOCK_SIZE * sizeof(uint32_t));
    codec.decodeArray(block, csize, outBlock, nvalue);

    uint32_t len = pool_.pool_[pSegment][pOffset + 5];

    if (len == 0)
        return 0;

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
    memset(outBlock, 0, BLOCK_SIZE * sizeof(uint32_t));
    codec.decodeArray(block, tfcsize, outBlock, nvalue);

    return pool_.pool_[pSegment][pOffset + 5];
}

uint32_t PositionalInvertedIndex::numberOfPositionBlocks_(size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_.pool_[pSegment][pOffset + csize + 7];
    return pool_.pool_[pSegment][pOffset + csize + tfcsize + 9];
}

uint32_t PositionalInvertedIndex::decompressPositionBlock_(
        FastPFor& codec,
        uint32_t* outBlock, size_t pointer) const
{
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    uint32_t csize = pool_.pool_[pSegment][pOffset + 6];
    uint32_t tfcsize = pool_.pool_[pSegment][pOffset + csize + 7];
    uint32_t nb = pool_.pool_[pSegment][pOffset + csize + tfcsize + 9];

    uint32_t index = pOffset + csize + tfcsize + 10;
    for (uint32_t i = 0; i < nb; ++i)
    {
        uint32_t sb = pool_.pool_[pSegment][index];
        size_t nvalue = BLOCK_SIZE;
        codec.decodeArray(&pool_.pool_[pSegment][index + 1], sb, &outBlock[i * BLOCK_SIZE], nvalue);
        index += sb + 1;
    }
    return pool_.pool_[pSegment][pOffset + csize + tfcsize + 8];
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

    uint32_t pos = pOffset + csize + tfcsize + 10;
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

bool PositionalInvertedIndex::containsDocid_(uint32_t docid, size_t& pointer) const
{
    if (pointer == UNDEFINED_POINTER)
    {
        return false;
    }
    uint32_t pSegment = DECODE_SEGMENT(pointer);
    uint32_t pOffset = DECODE_OFFSET(pointer);

    while (LESS_THAN(pool_.pool_[pSegment][pOffset + 3], docid, pool_.reverse_))
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
        uint32_t hits,
        std::vector<uint32_t>& docid_list) const
{
    docid_list.reserve(hits);
    FastPFor codec;
    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);

    while (headPointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = decompressDocidBlock_(codec, &docid_block[0], headPointers[0]);
        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t pivot = docid_block[i];
            bool found = true;
            for (uint32_t j = 1; j < headPointers.size(); ++j)
            {
                if (headPointers[j] == UNDEFINED_POINTER)
                    return;

                if (!containsDocid_(pivot, headPointers[j]))
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
        headPointers[0] = pool_.nextPointer(headPointers[0]);
    }
}

void PositionalInvertedIndex::bwandOr_(
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
        uint32_t count = decompressDocidBlock_(codec, &docid_block[0], headPointers[0]);

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t pivot = docid_block[i];
            float score = UB[0];
            for (uint32_t j = 1; j < headPointers.size(); ++j)
            {
                if (containsDocid_(pivot, headPointers[j]))
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

        headPointers[0] = pool_.nextPointer(headPointers[0]);
    }

    for (uint32_t i = 0; i < result_list.size(); ++i)
    {
        score_list.push_back(result_list[i].first);
        docid_list.push_back(result_list[i].second);
    }
}

bool PositionalInvertedIndex::gallopSearch_(
        FastPFor& codec,
        std::vector<uint32_t>& blockDocid,
        uint32_t& count, uint32_t& index, size_t& pointer,
        uint32_t pivot) const
{
    if (LESS_THAN(blockDocid[count - 1], pivot, pool_.reverse_))
    {
        if ((pointer = pool_.nextPointer(pointer, pivot)) == UNDEFINED_POINTER)
            return false;

        count = decompressDocidBlock_(codec, &blockDocid[0], pointer);
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

void PositionalInvertedIndex::wand_(
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
        counts[i] = decompressDocidBlock_(codec, &blockDocid[i][0], headPointers[i]);
        if (hasTf)
        {
            blockTf[i].resize(BLOCK_SIZE);
            decompressTfBlock_(codec, &blockTf[i][0], headPointers[i]);
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
                             pool_.reverse_))
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
                    if ((headPointers[aterm] = pool_.nextPointer(headPointers[aterm])) == UNDEFINED_POINTER)
                    {
                        mapping.erase(mapping.begin() + atermIdx);
                        --len;
                        --atermIdx;
                        continue;
                    }
                    else
                    {
                        counts[aterm] = decompressDocidBlock_(codec, &blockDocid[aterm][0], headPointers[aterm]);
                        if (hasTf)
                        {
                            decompressTfBlock_(codec, &blockTf[aterm][0], headPointers[aterm]);
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
                    decompressTfBlock_(codec, &blockTf[aterm][0], headPointers[aterm]);
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
                                 pool_.reverse_))
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
        size_t pointer0, size_t pointer1,
        std::vector<uint32_t>& docid_list) const
{
    std::vector<uint32_t> data0(BLOCK_SIZE);
    std::vector<uint32_t> data1(BLOCK_SIZE);

    uint32_t c0 = decompressDocidBlock_(codec, &data0[0], pointer0);
    uint32_t c1 = decompressDocidBlock_(codec, &data1[0], pointer1);
    uint32_t i0 = 0, i1 = 0;

    while (true)
    {
        if (data1[i1] == data0[i0])
        {
            docid_list.push_back(data0[i0]);
            if (++i0 == c0)
            {
                if ((pointer0 = pool_.nextPointer(pointer0)) == UNDEFINED_POINTER)
                    break;

                c0 = decompressDocidBlock_(codec, &data0[0], pointer0);
                i0 = 0;
            }
            if (++i1 == c1)
            {
                if ((pointer1 = pool_.nextPointer(pointer1)) == UNDEFINED_POINTER)
                    break;

                c1 = decompressDocidBlock_(codec, &data1[0], pointer1);
                i1 = 0;
            }
        }

        if (LESS_THAN(data0[i0], data1[i1], pool_.reverse_))
        {
            if (!gallopSearch_(codec, data0, c0, ++i0, pointer0, data1[i1]))
                break;
        }
        else if (LESS_THAN(data1[i1], data0[i0], pool_.reverse_))
        {
            if (!gallopSearch_(codec, data1, c1, ++i1, pointer1, data0[i0]))
                break;
        }
    }
}

void PositionalInvertedIndex::intersectSetPostingsList_(
        FastPFor& codec,
        size_t pointer,
        std::vector<uint32_t>& docid_list) const
{
    std::vector<uint32_t> blockDocid(BLOCK_SIZE);
    uint32_t c = decompressDocidBlock_(codec, &blockDocid[0], pointer);
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
                if ((pointer = pool_.nextPointer(pointer)) == UNDEFINED_POINTER)
                    break;

                c = decompressDocidBlock_(codec, &blockDocid[0], pointer);
                i = 0;
            }
        }
        else if (LESS_THAN(blockDocid[i], docid_list[iCurrent], pool_.reverse_))
        {
            if (!gallopSearch_(codec, blockDocid, c, ++i, pointer, docid_list[iCurrent]))
                break;
        }
        else
        {
            while (iCurrent < docid_list.size() && LESS_THAN(docid_list[iCurrent], blockDocid[i], pool_.reverse_))
            {
                ++iCurrent;
            }
            if (iCurrent == docid_list.size())
                break;
        }
    }

    docid_list.resize(iSet);
}

void PositionalInvertedIndex::intersectSvS_(
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
            uint32_t c = decompressDocidBlock_(codec, &block[0], t);
            uint32_t r = iSet + c <= length ? c : length - iSet;
            memcpy(&docid_list[iSet], &block[0], r * sizeof(uint32_t));
            iSet += r;
            t = pool_.nextPointer(t);
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
