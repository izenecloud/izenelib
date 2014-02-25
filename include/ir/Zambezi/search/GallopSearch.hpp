#ifndef IZENELIB_IR_ZAMBEZI_SEARCH_GALLOP_SEARCH_HPP
#define IZENELIB_IR_ZAMBEZI_SEARCH_GALLOP_SEARCH_HPP

#include "../Consts.hpp"

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

uint32_t gallopSearch(
        const uint32_t* block,
        bool reverse,
        uint32_t count,
        uint32_t index,
        uint32_t pivot);

}

NS_IZENELIB_IR_END

#endif
