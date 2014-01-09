#ifndef IZENELIB_IR_ZAMBEZI_FILTER_BASE_HPP
#define IZENELIB_IR_ZAMBEZI_FILTER_BASE_HPP

#include "Consts.hpp"


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class FilterBase
{
public:
    FilterBase() : iterable_(false), reverse_(false) {}
    virtual ~FilterBase() {}

    virtual bool test(uint32_t id) const { return true; }
    virtual uint32_t find_first() const { return reverse_ ? INVALID_ID : 1; }
    virtual uint32_t find_next(uint32_t id) const { return reverse_ ? id - 1 : id + 1; }

    bool iterable() const { return iterable_; }
    bool reverse() const { return reverse_; }

protected:
    bool iterable_;
    bool reverse_;
};

}

NS_IZENELIB_IR_END

#endif
