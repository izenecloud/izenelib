#ifndef IZENELIB_IR_ZAMBEZI_PREDICATE_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_PREDICATE_INVERTED_INDEX_HPP

#include "SegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
//#include "buffer/PredicateBufferMaps.hpp"
#include "Consts.hpp"
#include <util/compression/int/fastpfor/fastpfor.h>

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class PredicateInvertedIndex
{
public:
    PredicateInvertedIndex(
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            bool reverse = true);

    ~PredicateInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

//  void insertConjunction();

//  void flush();

    uint32_t totalDocNum() const;

//  void retrieval() const;

private:
//  PredicateBufferMaps buffer_;
    SegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    FastPFor codec_;
};

}

NS_IZENELIB_IR_END

#endif
