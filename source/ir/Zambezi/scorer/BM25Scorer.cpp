#include <ir/Zambezi/scorer/BM25Scorer.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <cmath>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace details
{

    float bm25tf(uint32_t tf, uint32_t docLen, float avgDocLen, float K1, float B)
    {
        return (1.0f + K1) * tf / (K1 * (1.0f - B + B * docLen / avgDocLen) + tf);
    }

    float bm25(uint32_t tf, uint32_t df, uint32_t numDocs, uint32_t docLen, float avgDocLen, float K1, float B)
    {
        return bm25tf(tf, docLen, avgDocLen, K1, B) * idf(numDocs, df);
    }

    float bm25Phrase(uint32_t tf, uint32_t docLen, float avgDocLen, float defaultIdf, float K1, float B)
    {
        return bm25tf(tf, docLen, avgDocLen, K1, B) * defaultIdf;
    }

}

BM25Scorer::BM25Scorer(float K1, float B)
    : Scorer(Scorer::BM25)
    , K1_(K1)
    , B_(B)
{
}

float BM25Scorer::computeTermScore(
        const Pointers& pointers, uint32_t query, uint32_t docid, uint32_t tf) const
{
    return details::bm25(
            tf,
            pointers.df.get(query),
            pointers.totalDocs,
            pointers.docLen.get(docid),
            pointers.totalDocLen / (float) pointers.totalDocs,
            K1_, B_);
}

float BM25Scorer::computePhraseScore(
        const Pointers& pointers, uint32_t docid, uint32_t tf) const
{
    return details::bm25Phrase(
            tf,
            pointers.docLen.get(docid),
            pointers.totalDocLen / (float) pointers.totalDocs,
            pointers.defaultIdf,
            K1_, B_);
}

}

NS_IZENELIB_IR_END
