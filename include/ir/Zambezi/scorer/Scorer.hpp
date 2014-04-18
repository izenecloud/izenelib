#ifndef IZENELIB_IR_ZAMBEZI_SCORE_SCORER_HPP
#define IZENELIB_IR_ZAMBEZI_SCORE_SCORER_HPP

#include "../Pointers.hpp"

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class Scorer
{
public:
    enum FunctionName
    {
        BM25 = 0,
        DIRICHLET = 1,
    };

    Scorer(FunctionName func)
        : function(func)
    {
    }

    virtual float computeTermScore(
            const Pointers& pointers,
            uint32_t query, uint32_t docid, uint32_t tf) const = 0;

    virtual float computePhraseScore(
            const Pointers& pointers,
            uint32_t docid, uint32_t tf) const = 0;

public:
    FunctionName function;
};

}

NS_IZENELIB_IR_END

#endif
