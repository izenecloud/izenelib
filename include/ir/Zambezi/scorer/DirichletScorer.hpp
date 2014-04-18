#ifndef DIRICHLET_H_GUARD
#define DIRICHLET_H_GUARD

#include "Scorer.hpp"

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class DirichletScorer : public Scorer
{
public:
    DirichletScorer(float MU = 1000.0f);

    float computeTermScore(
            const Pointers& pointers,
            uint32_t query, uint32_t docid, uint32_t tf) const;

    float computePhraseScore(
            const Pointers& pointers,
            uint32_t docid, uint32_t tf) const;

private:
    float MU_;
};

}

NS_IZENELIB_IR_END

#endif
