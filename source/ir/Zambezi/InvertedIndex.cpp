#include <ir/Zambezi/InvertedIndex.hpp>
#include <ir/Zambezi/intersection/Algorithms.hpp>
#include <ir/Zambezi/Utils.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace detail
{
}

InvertedIndex::InvertedIndex(
        IndexType type,
        bool reverse, bool bloomEnabled,
        uint32_t nbHash, uint32_t bitsPerElement)
    : type_(type)
    , buffer_(DEFAULT_VOCAB_SIZE, type)
    , pool_(NUMBER_OF_POOLS, reverse, bloomEnabled, nbHash, bitsPerElement)
    , pointers_(DEFAULT_VOCAB_SIZE)
{
}

InvertedIndex::~InvertedIndex()
{
}

void InvertedIndex::save(std::ostream& ostr) const
{
    buffer_.save(ostr);
    pool_.save(ostr);
    dictionary_.save(ostr);
    pointers_.save(ostr);
}

void InvertedIndex::load(std::istream& istr)
{
    buffer_.load(istr);
    pool_.load(istr);
    dictionary_.load(istr);
    pointers_.load(istr);
}

bool InvertedIndex::hasValidPostingsList(uint32_t termid) const
{
    return pointers_.getHeadPointer(termid) != UNDEFINED_POINTER;
}

void InvertedIndex::insertDoc(uint32_t docid, const std::vector<std::string>& term_list)
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

            if (posBuffer.capacity() == posBuffer.size())
            {
                uint32_t len = posBuffer.capacity();
                posBuffer.reserve(len * 2);
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
    pointers_.totalDocLen_ += term_list.size();
    ++pointers_.totalDocs_;

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

        if (type_ == POSITIONAL)
        {
            if (docBuffer.size() % BLOCK_SIZE == 0)
            {
                buffer_.posBlockHead_[id] = posBuffer.size();
                posBuffer.push_back(0);
            }
        }

        if (docBuffer.size() == docBuffer.capacity())
        {
            uint32_t nb = docBuffer.size() / BLOCK_SIZE;
            size_t pointer = buffer_.tailPointer_[id];
            uint32_t ps = 0;
            for (uint32_t j = 0; j < nb; ++j)
            {
                switch (type_)
                {
                    case NON_POSITIONAL:
                        pointer = pool_.compressAndAddNonPositional(
                                &docBuffer[j * BLOCK_SIZE],
                                BLOCK_SIZE,
                                pointer);
                        break;

                    case TF_ONLY:
                        pointer = pool_.compressAndAddTfOnly(
                                &docBuffer[j * BLOCK_SIZE],
                                &tfBuffer[j * BLOCK_SIZE],
                                BLOCK_SIZE,
                                pointer);
                        break;

                    case POSITIONAL:
                        pointer = pool_.compressAndAddPositional(
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

                if (pool_.reverse_ || pointers_.getHeadPointer(id) == UNDEFINED_POINTER)
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

            if (docBuffer.size() < MAX_BLOCK_SIZE)
            {
                uint32_t newLen = docBuffer.size() * EXPANSION_RATE;
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

void InvertedIndex::flush()
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
                pointer = pool_.compressAndAddNonPositional(
                        &docBuffer[i * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);
                break;

            case TF_ONLY:
                pointer = pool_.compressAndAddTfOnly(
                        &docBuffer[i * BLOCK_SIZE],
                        &tfBuffer[i * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);
                break;

            case POSITIONAL:
                pointer = pool_.compressAndAddPositional(
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

            if (pool_.reverse_ || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
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
                        &docBuffer[nb * BLOCK_SIZE],
                        res,
                        pointer);
                break;

            case TF_ONLY:
                pointer = pool_.compressAndAddTfOnly(
                        &docBuffer[nb * BLOCK_SIZE],
                        &tfBuffer[nb * BLOCK_SIZE],
                        res,
                        pointer);
                break;

            case POSITIONAL:
                pointer = pool_.compressAndAddPositional(
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

            if (pool_.reverse_ || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
            }
        }
    }
}

void InvertedIndex::retrieval(
        Algorithm algorithm,
        const std::vector<std::string>& term_list,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    std::vector<uint32_t> queries;
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getId(term_list[i]);
        if (termid != INVALID_ID && pointers_.getHeadPointer(termid) != UNDEFINED_POINTER)
        {
            queries.push_back(termid);
        }
    }

    uint32_t qlen = queries.size();
    std::vector<uint32_t> qdf(qlen);
    std::vector<uint32_t> sortedDfIndex(qlen);
    std::vector<size_t> qHeadPointers(qlen);

    uint32_t minimumDf = -1;
    for (uint32_t i = 0; i < qlen; ++i)
    {
        qdf[i] = pointers_.getDf(queries[i]);
        if (qdf[i] < minimumDf)
        {
            minimumDf = qdf[i];
        }
    }

    if (algorithm == BWAND_OR || algorithm == BWAND_AND || algorithm == SVS)
    {
        for (uint32_t i = 0; i < qlen; ++i)
        {
            uint32_t minDf = 0xFFFFFFFF;
            for (uint32_t j = 0; j < qlen; ++j)
            {
                if (qdf[j] < minDf)
                {
                    minDf = qdf[j];
                    sortedDfIndex[i] = j;
                }
            }
            qdf[sortedDfIndex[i]] = 0xFFFFFFFF;
        }
    }
    else
    {
        for (uint32_t i = 0; i < qlen; ++i)
        {
            sortedDfIndex[i] = i;
        }
    }

    for (uint32_t i = 0; i < qlen; ++i)
    {
        qHeadPointers[i] = pointers_.getHeadPointer(queries[sortedDfIndex[i]]);
        qdf[i] = pointers_.getDf(queries[sortedDfIndex[i]]);
    }

    if (algorithm == BWAND_OR)
    {
        std::vector<float> UB(qlen);
        for (uint32_t i = 0; i < qlen; ++i)
        {
            UB[i] = idf(pointers_.totalDocs_, qdf[i]);
        }
        Intersection::bwandOr(pool_, qHeadPointers, UB, hits, docid_list, score_list);
    }
    else if (algorithm == BWAND_AND)
    {
        if (!hits)
        {
            hits = minimumDf;
        }
        Intersection::bwandAnd(pool_, qHeadPointers, hits, docid_list);
    }
//  else if (algorithm == WAND || algorithm == MBWAND)
//  {
//      std::vector<float> UB(qlen);
//      for (i = 0; i < qlen; ++i)
//      {
//          if (algorithm == WAND)
//          {
//              UB[i] = default_bm25(
//                      pointers_.getMaxTf(queries[sortedDfIndex[i]]),
//                      qdf[i],
//                      pointers_.totalDocs_,
//                      pointers_.getMaxTfDocLen(queries[sortedDfIndex[i]]),
//                      pointers_.totalDocLen_ / (float)pointers_.totalDocs_);
//          }
//          else
//          {
//              UB[i] = idf(pointers_.totalDocs_, qdf[i]);
//          }
//      }
//      Intersection::wand(
//              pool_, qHeadPointers, qdf, UB, qlen,
//              pointers_.docLen_,
//              pointers_.totalDocs_,
//              pointers_.totalDocLen_ / (float)pointers_.totalDocs_,
//              hits, algorithm == MBWAND, docid_list, score_list);
//  }
//  else if (algorithm == SVS)
//  {
//      if (!hitsSpecified)
//      {
//          hits = minimumDf;
//      }
//      Intersection::intersectSvS(pool_, qHeadPointers, qlen, minimumDf, hits, docid_list);
//  }
}

}

NS_IZENELIB_IR_END
