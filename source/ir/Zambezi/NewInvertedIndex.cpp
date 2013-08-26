#include <ir/Zambezi/NewInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace detail
{
}

NewInvertedIndex::NewInvertedIndex(
        bool reverse, bool bloomEnabled,
        uint32_t nbHash, uint32_t bitsPerElement)
    : buffer_(DEFAULT_VOCAB_SIZE)
    , pool_(NUMBER_OF_POOLS, reverse, bloomEnabled, nbHash, bitsPerElement)
    , pointers_(DEFAULT_VOCAB_SIZE)
{
}

NewInvertedIndex::~NewInvertedIndex()
{
}

void NewInvertedIndex::save(std::ostream& ostr) const
{
    buffer_.save(ostr);
    pool_.save(ostr);
    dictionary_.save(ostr);
    pointers_.save(ostr);
}

void NewInvertedIndex::load(std::istream& istr)
{
    buffer_.load(istr);
    pool_.load(istr);
    dictionary_.load(istr);
    pointers_.load(istr);
}

bool NewInvertedIndex::hasValidPostingsList(uint32_t termid) const
{
    return pointers_.getHeadPointer(termid) != UNDEFINED_POINTER;
}

void NewInvertedIndex::insertDoc(
        uint32_t docid,
        const std::vector<uint32_t>& attr_score_list,
        const std::vector<std::vector<std::string> >& attr_term_list)
{
    std::map<uint32_t, uint32_t> unique_term_list;
    uint32_t total_len = 0;

    for (uint32_t i = 0; i < attr_term_list.size(); ++i)
    {
        total_len += attr_term_list[i].size();
        std::set<uint32_t> tmp_term_list;
        for (std::vector<std::string>::const_iterator it = attr_term_list[i].begin();
                it != attr_term_list[i].end(); ++it)
        {
            uint32_t id = dictionary_.insertTerm(*it);
            if (tmp_term_list.insert(id).second)
                unique_term_list[id] += attr_score_list[i];
            pointers_.cf_.increment(id);
        }
    }

    pointers_.setDocLen(docid, total_len);

    for (std::map<uint32_t, uint32_t>::const_iterator it = unique_term_list.begin();
            it != unique_term_list.end(); ++it)
    {
        uint32_t id = it->first;
        uint32_t score = it->second;
        std::vector<uint32_t>& docBuffer = buffer_.getDocidList(id);
        std::vector<uint32_t>& scoreBuffer = buffer_.getScoreList(id);

        uint32_t df = pointers_.getDf(id);
        if (df < DF_CUTOFF)
        {
            if (docBuffer.capacity() == 0)
            {
                docBuffer.reserve(DF_CUTOFF);
                scoreBuffer.reserve(DF_CUTOFF);
            }
            docBuffer.push_back(docid);
            scoreBuffer.push_back(score);
            pointers_.df_.increment(id);
            continue;
        }

        if (docBuffer.capacity() < BLOCK_SIZE)
        {
            docBuffer.reserve(BLOCK_SIZE);
            scoreBuffer.reserve(BLOCK_SIZE);
        }

        docBuffer.push_back(docid);
        scoreBuffer.push_back(score);
        pointers_.df_.increment(id);

        if (docBuffer.size() == docBuffer.capacity())
        {
            uint32_t nb = docBuffer.size() / BLOCK_SIZE;
            size_t pointer = buffer_.tailPointer_[id];
            for (uint32_t j = 0; j < nb; ++j)
            {
                pointer = pool_.compressAndAppend(
                        &docBuffer[j * BLOCK_SIZE],
                        &scoreBuffer[j * BLOCK_SIZE],
                        BLOCK_SIZE,
                        pointer);

                if (pool_.reverse_ || pointers_.getHeadPointer(id) == UNDEFINED_POINTER)
                {
                    pointers_.setHeadPointer(id, pointer);
                }
            }
            buffer_.tailPointer_[id] = pointer;

            docBuffer.clear();
            scoreBuffer.clear();

            if (scoreBuffer.capacity() < MAX_BLOCK_SIZE)
            {
                uint32_t newLen = docBuffer.capacity() * EXPANSION_RATE;
                docBuffer.reserve(newLen);
            }
        }
    }
}

void NewInvertedIndex::flush()
{
    uint32_t term = UNDEFINED_OFFSET;
    while ((term = buffer_.nextIndex(term, DF_CUTOFF)) != UNDEFINED_OFFSET)
    {
        std::vector<uint32_t>& docBuffer = buffer_.docid_[term];
        std::vector<uint32_t>& scoreBuffer = buffer_.score_[term];

        uint32_t pos = docBuffer.size();
        size_t pointer = buffer_.tailPointer_[term];

        uint32_t nb = pos / BLOCK_SIZE;
        uint32_t res = pos % BLOCK_SIZE;

        for (uint32_t i = 0; i < nb; ++i)
        {
            pointer = pool_.compressAndAppend(
                    &docBuffer[i * BLOCK_SIZE],
                    &scoreBuffer[nb * BLOCK_SIZE],
                    BLOCK_SIZE,
                    pointer);

            if (pool_.reverse_ || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
            }
        }

        if (res > 0)
        {
            pointer = pool_.compressAndAppend(
                    &docBuffer[nb * BLOCK_SIZE],
                    &scoreBuffer[nb * BLOCK_SIZE],
                    res,
                    pointer);

            if (pool_.reverse_ || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
            }
        }
    }
}

void NewInvertedIndex::retrieval(
        const std::vector<std::string>& term_list,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list) const
{
    std::vector<uint32_t> queries;
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getTermId(term_list[i]);
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

    for (uint32_t i = 0; i < qlen; ++i)
    {
        qHeadPointers[i] = pointers_.getHeadPointer(queries[sortedDfIndex[i]]);
        qdf[i] = pointers_.getDf(queries[sortedDfIndex[i]]);
    }

//  pool_.intersectSvS(qHeadPointers, minimumDf, hits, docid_list);
}

}

NS_IZENELIB_IR_END
