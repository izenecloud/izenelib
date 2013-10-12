#include <ir/Zambezi/BEInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

BEInvertedIndex::BEInvertedIndex(
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        bool reverse)
    : buffer_(DEFAULT_VOCAB_SIZE)
    , pool_(maxPoolSize, numberOfPools, reverse)
    , pointers_(DEFAULT_VOCAB_SIZE)
{
}

BEInvertedIndex::~BEInvertedIndex()
{
}

void BEInvertedIndex::save(std::ostream& ostr) const
{
    buffer_.save(ostr);
    pool_.save(ostr);
    dictionary_.save(ostr);
    pointers_.save(ostr);
}

void BEInvertedIndex::load(std::istream& istr)
{
    buffer_.load(istr);
    pool_.load(istr);
    dictionary_.load(istr);
    pointers_.load(istr);
}

bool BEInvertedIndex::hasValidPostingsList(uint32_t termid) const
{
    return pointers_.getHeadPointer(termid) != UNDEFINED_POINTER;
}

void BEInvertedIndex::insertConjunction(
        uint32_t conj_id,
        const std::vector<std::string>& term_list,
        const std::vector<uint32_t>& score_list)
{
    pointers_.setDocLen(conj_id, term_list.size());

    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t id = dictionary_.insertTerm(term_list[i]);
        pointers_.cf_.increment(id);
        std::vector<uint32_t>& docBuffer = buffer_.getDocidList(id);
        std::vector<uint32_t>& scoreBuffer = buffer_.getScoreList(id);

        if (pointers_.getDf(id) < DF_CUTOFF)
        {
            if (docBuffer.capacity() == 0)
            {
                docBuffer.reserve(DF_CUTOFF);
                scoreBuffer.reserve(DF_CUTOFF);
            }
            docBuffer.push_back(conj_id);
            scoreBuffer.push_back(score_list[i]);
            pointers_.df_.increment(id);
            continue;
        }

        if (docBuffer.capacity() < BLOCK_SIZE)
        {
            docBuffer.reserve(BLOCK_SIZE);
            scoreBuffer.reserve(BLOCK_SIZE);
        }

        docBuffer.push_back(conj_id);
        scoreBuffer.push_back(score_list[i]);
        pointers_.df_.increment(id);

        if (docBuffer.size() == docBuffer.capacity())
        {
            uint32_t nb = docBuffer.size() / BLOCK_SIZE;
            size_t pointer = buffer_.tailPointer_[id];

            for (uint32_t j = 0; j < nb; ++j)
            {
                pointer = pool_.compressAndAppend(
                        codec_,
                        &docBuffer[j * BLOCK_SIZE],
                        &scoreBuffer[j * BLOCK_SIZE],
                        NULL,
                        BLOCK_SIZE,
                        pointer);

                if (pool_.isReverse() || pointers_.getHeadPointer(id) == UNDEFINED_POINTER)
                {
                    pointers_.setHeadPointer(id, pointer);
                }
            }

            buffer_.tailPointer_[id] = pointer;

            docBuffer.clear();
            scoreBuffer.clear();

            if (docBuffer.capacity() < MAX_BLOCK_SIZE)
            {
                uint32_t newLen = docBuffer.capacity() * EXPANSION_RATE;
                docBuffer.reserve(newLen);
                scoreBuffer.reserve(newLen);
            }
        }
    }
}

void BEInvertedIndex::flush()
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
                    codec_,
                    &docBuffer[i * BLOCK_SIZE],
                    &scoreBuffer[i * BLOCK_SIZE],
                    NULL,
                    BLOCK_SIZE,
                    pointer);

            if (pool_.isReverse() || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
            }
        }

        if (res > 0)
        {
            pointer = pool_.compressAndAppend(
                    codec_,
                    &docBuffer[nb * BLOCK_SIZE],
                    &scoreBuffer[nb * BLOCK_SIZE],
                    NULL,
                    res,
                    pointer);

            if (pool_.isReverse() || pointers_.getHeadPointer(term) == UNDEFINED_POINTER)
            {
                pointers_.setHeadPointer(term, pointer);
            }
        }

        buffer_.tailPointer_[term] = pointer;

        docBuffer.clear();
        scoreBuffer.clear();
    }
}

uint32_t BEInvertedIndex::totalDocNum() const
{
    return pointers_.getTotalDocs();
}

void BEInvertedIndex::retrieval(
        Algorithm algorithm,
        const std::vector<std::string>& term_list,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<uint32_t>& score_list) const
{
    std::vector<std::pair<uint32_t, size_t> > queries;
    uint32_t minimumDf = 0xFFFFFFFF;
    for (uint32_t i = 0; i < term_list.size(); ++i)
    {
        uint32_t termid = dictionary_.getTermId(term_list[i]);
        if (termid != INVALID_ID)
        {
            size_t pointer = pointers_.getHeadPointer(termid);
            if (pointer != UNDEFINED_POINTER)
            {
                queries.push_back(std::make_pair(pointers_.getDf(termid), pointer));
                minimumDf = std::min(queries.back().first, minimumDf);
            }
        }
    }

    if (queries.empty()) return;

    if (algorithm == SVS)
    {
        std::sort(queries.begin(), queries.end());
    }

    std::vector<size_t> qHeadPointers(queries.size());
    for (uint32_t i = 0; i < queries.size(); ++i)
    {
        qHeadPointers[i] = queries[i].second;
    }
}

}

NS_IZENELIB_IR_END
