#include <ir/Zambezi/scorer/DirichletScorer.hpp>

#include <cmath>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

namespace details
{

    float termBackgroundProb(long cf, long collectionLength)
    {
        return (float) ((double)cf / collectionLength);
    }

    float dirichlet(uint32_t tf, uint32_t docLen, long cf, long collectionLength, float MU)
    {
        return logf(((float)tf + MU * termBackgroundProb(cf, collectionLength)) /
                (docLen + MU));
    }

    float dirichletPhrase(uint32_t tf, uint32_t docLen, long defaultCf,
            long collectionLength, float MU)
    {
        return dirichlet(tf, docLen, defaultCf, collectionLength, MU);
    }

}

DirichletScorer::DirichletScorer(float MU)
    : Scorer(Scorer::DIRICHLET)
    , MU_(MU)
{
}

float DirichletScorer::computeTermScore(
        const Pointers& pointers,
        uint32_t query, uint32_t docid, uint32_t tf) const
{
    return details::dirichlet(
            tf, pointers.docLen.get(docid),
            pointers.cf.get(query), pointers.totalDocLen,
            MU_);
}

float DirichletScorer::computePhraseScore(
        const Pointers& pointers,
        uint32_t docid, uint32_t tf) const
{
    return details::dirichlet(
            tf, pointers.docLen.get(docid),
            pointers.defaultCf, pointers.totalDocLen,
            MU_);
}

}

NS_IZENELIB_IR_END
