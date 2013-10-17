#ifndef IZENELIB_IR_ZAMBEZI_BOOL_EXP_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_BOOL_EXP_INVERTED_INDEX_HPP

#include "BoolExpSegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/BoolExpBufferMaps.hpp"
#include "Consts.hpp"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class BoolExpInvertedIndex
{
public:
    BoolExpInvertedIndex(
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            bool reverse = true);

    ~BoolExpInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    bool hasValidPostingsList(uint32_t termid) const;

    void insertConjunction(
            uint32_t conj_id,
            const std::vector<std::string>& pred_list,
            const std::vector<uint32_t>& score_list);

    void flush();

    uint32_t totalDocNum() const;

    void retrieval(
            Algorithm algorithm,
            const std::vector<std::string>& term_list,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

private:
    BoolExpBufferMaps buffer_;
    BoolExpSegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    FastPFor codec_;
};

}

NS_IZENELIB_IR_END

#endif
