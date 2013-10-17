#ifndef IZENELIB_IR_ZAMBEZI_POSITIONAL_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_POSITIONAL_INVERTED_INDEX_HPP

#include "PositionalSegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/PositionalBufferMaps.hpp"
#include "Consts.hpp"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class PositionalInvertedIndex
{
public:
    PositionalInvertedIndex(
            IndexType type = NON_POSITIONAL,
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            bool reverse = true,
            bool bloomEnabled = true,
            uint32_t nbHash = 3,
            uint32_t bitsPerElement = 8);

    ~PositionalInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    void insertDoc(uint32_t docid, const std::vector<std::string>& term_list);
    void flush();

    void retrieval(
            Algorithm algorithm,
            const std::vector<std::string>& term_list,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

private:
    IndexType type_;
    PositionalBufferMaps buffer_;
    PositionalSegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    FastPFor codec_;
};

}

NS_IZENELIB_IR_END

#endif
