#include <ir/Zambezi/ConjunctionInvertedIndex.hpp>
#include <ir/Zambezi/Utils.hpp>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

ConjunctionInvertedIndex::ConjunctionInvertedIndex(
        uint32_t maxPoolSize,
        uint32_t numberOfPools,
        bool reverse)
//  : buffer_(DEFAULT_VOCAB_SIZE)
    : pool_(maxPoolSize, numberOfPools, reverse)
    , dictionary_(DEFAULT_VOCAB_SIZE)
    , pointers_(DEFAULT_VOCAB_SIZE, 0)
{
}

ConjunctionInvertedIndex::~ConjunctionInvertedIndex()
{
}

void ConjunctionInvertedIndex::save(std::ostream& ostr) const
{
//  buffer_.save(ostr);
    pool_.save(ostr);
    dictionary_.save(ostr);
    pointers_.save(ostr);
}

void ConjunctionInvertedIndex::load(std::istream& istr)
{
//  buffer_.load(istr);
    pool_.load(istr);
    dictionary_.load(istr);
    pointers_.load(istr);
}

uint32_t ConjunctionInvertedIndex::totalDocNum() const
{
    return pointers_.totalDocs_;
}

}

NS_IZENELIB_IR_END
