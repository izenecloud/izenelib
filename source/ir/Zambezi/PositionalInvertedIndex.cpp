#include <ir/Zambezi/PositionalInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>

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
    , pool_(maxPoolSize, numberOfPools, reverse, bloomEnabled, nbHash, bitsPerElement)
    , pointers_(DEFAULT_VOCAB_SIZE)
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
}

void PositionalInvertedIndex::load(std::istream& istr)
{
    buffer_.load(istr);
    pool_.load(istr);
    dictionary_.load(istr);
    pointers_.load(istr);
}

bool PositionalInvertedIndex::hasValidPostingsList(uint32_t termid) const
{
    return pointers_.getHeadPointer(termid) != UNDEFINED_POINTER;
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
            std::vector<uint32_t>& tfBuffer = buffer_.getTfList(id);
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
            std::vector<uint32_t>& tfBuffer = buffer_.getTfList(id);
            std::vector<uint32_t>& posBuffer = buffer_.getPositionList(id);

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

    pointers_.setDocLen(docid, term_list.size());

    for (std::set<uint32_t>::const_iterator it = uniqueTerms.begin();
            it != uniqueTerms.end(); ++it)
    {
        uint32_t id = *it;
        std::vector<uint32_t>& docBuffer = buffer_.getDocidList(id);
        std::vector<uint32_t>& tfBuffer = buffer_.getTfList(id);
        std::vector<uint32_t>& posBuffer = buffer_.getPositionList(id);

        if (type_ != NON_POSITIONAL)
        {
            uint32_t tf = tfBuffer.back();
            uint32_t dl = pointers_.getDocLen(docid);
            float bm25TfScore = default_bm25tf(
                    tf, dl,
                    pointers_.totalDocLen_ /
                    (float)pointers_.totalDocs_);
            float maxBm25TfScore = default_bm25tf(
                    pointers_.getMaxTf(id),
                    pointers_.getMaxTfDocLen(id),
                    pointers_.totalDocLen_ /
                    (float)pointers_.totalDocs_);
            if (bm25TfScore > maxBm25TfScore)
            {
                pointers_.setMaxTf(id, tf, dl);
            }
        }

        uint32_t df = pointers_.getDf(id);
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
                        pointer = pool_.compressAndAddNonPositional(
                                codec_,
                                &docBuffer[j * BLOCK_SIZE],
                                BLOCK_SIZE,
                                pointer);
                        break;

                    case TF_ONLY:
                        pointer = pool_.compressAndAddTfOnly(
                                codec_,
                                &docBuffer[j * BLOCK_SIZE],
                                &tfBuffer[j * BLOCK_SIZE],
                                BLOCK_SIZE,
                                pointer);
                        break;

                    case POSITIONAL:
                        pointer = pool_.compressAndAddPositional(
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

                if (pool_.isReverse() || pointers_.getHeadPointer(id) == UNDEFINED_POINTER)
                {
                    pointers_.setHeadPointer(id, pointer);
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
        std::vector<uint32_t>& docBuffer = buffer_.getDocidList(term);
        std::vector<uint32_t>& tfBuffer = buffer_.getTfList(term);
        std::vector<uint32_t>& posBuffer = buffer_.getPositionList(term);

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
                pointer = pool_.compressAndAddNonPositional(
                        codec_,
                        &docBuffer[i * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);
                break;

            case TF_ONLY:
                pointer = pool_.compressAndAddTfOnly(
                        codec_,
                        &docBuffer[i * BLOCK_SIZE],
                        &tfBuffer[i * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);
                break;

            case POSITIONAL:
                pointer = pool_.compressAndAddPositional(
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

            if (pool_.isReverse() || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
            }
        }

        if (res > 0)
        {
            switch (type_)
            {
            case NON_POSITIONAL:
                pointer = pool_.compressAndAddNonPositional(
                        codec_,
                        &docBuffer[nb * BLOCK_SIZE],
                        res,
                        pointer);
                break;

            case TF_ONLY:
                pointer = pool_.compressAndAddTfOnly(
                        codec_,
                        &docBuffer[nb * BLOCK_SIZE],
                        &tfBuffer[nb * BLOCK_SIZE],
                        res,
                        pointer);
                break;

            case POSITIONAL:
                pointer = pool_.compressAndAddPositional(
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

            if (pool_.isReverse() || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
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
            size_t pointer = pointers_.getHeadPointer(termid);
            if (pointer != UNDEFINED_POINTER)
            {
                queries.push_back(boost::make_tuple(pointers_.getDf(termid), termid, pointer));
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
        pool_.bwandOr(qHeadPointers, UB, hits, docid_list, score_list);
    }
    else if (algorithm == BWAND_AND)
    {
        if (!hits)
        {
            hits = minimumDf;
        }
        pool_.bwandAnd(qHeadPointers, hits, docid_list);
    }
    else if (algorithm == WAND || algorithm == MBWAND)
    {
        std::vector<float> UB(queries.size());
        for (uint32_t i = 0; i < queries.size(); ++i)
        {
            if (algorithm == WAND)
            {
                UB[i] = default_bm25(
                        pointers_.getMaxTf(queries[i].get<1>()),
                        qdf[i],
                        pointers_.totalDocs_,
                        pointers_.getMaxTfDocLen(queries[i].get<1>()),
                        pointers_.totalDocLen_ / (float)pointers_.totalDocs_);
            }
            else
            {
                UB[i] = idf(pointers_.totalDocs_, qdf[i]);
            }
        }
        pool_.wand(
                qHeadPointers,
                qdf,
                UB,
                pointers_.docLen_.counter_,
                pointers_.totalDocs_,
                pointers_.totalDocLen_ / (float)pointers_.totalDocs_,
                hits,
                algorithm == WAND,
                docid_list,
                score_list);
    }
    else if (algorithm == SVS)
    {
        pool_.intersectSvS(qHeadPointers, minimumDf, hits, docid_list);
    }
}

}

NS_IZENELIB_IR_END
