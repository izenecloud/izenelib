#ifndef IZENELIB_IR_ZAMBEZI_FILTER_BASE_HPP
#define IZENELIB_IR_ZAMBEZI_FILTER_BASE_HPP

#include "Consts.hpp"


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class FilterBase
{
public:
    virtual ~FilterBase() {}

    virtual bool test(uint32_t id) const { return true; }
    virtual uint32_t find_first(bool reverse) const { return reverse ? std::numeric_limits<int>::max() : 1; }
    virtual uint32_t find_next(uint32_t id, bool reverse) const { return reverse ? id - 1 : id + 1; }
};

}

NS_IZENELIB_IR_END

#endif
