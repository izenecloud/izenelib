#ifndef IZENELIB_IR_ZAMBEZI_SCORER_BM25_HPP
#define IZENELIB_IR_ZAMBEZI_SCORER_BM25_HPP

#include "Scorer.hpp"

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class BM25Scorer : public Scorer
{
public:
    BM25Scorer(float K1 = 0.5f, float B = 0.3f);

    float computeTermScore(
            const Pointers& pointers,
            uint32_t query, uint32_t docid, uint32_t tf) const;

    float computePhraseScore(
            const Pointers& pointers,
            uint32_t docid, uint32_t tf) const;

private:
    float K1_;
    float B_;
};

}

NS_IZENELIB_IR_END

#endif
