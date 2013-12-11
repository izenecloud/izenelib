#ifndef IZENELIB_IR_ZAMBEZI_FILTER_HPP
#define IZENELIB_IR_ZAMBEZI_FILTER_HPP

#include "Consts.hpp"


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class ZambeziFilter
{
public:
    ZambeziFilter() : reverse_(false), iterable_(false) {}
    virtual ~ZambeziFilter() {}

    virtual bool test(uint32_t id) const { return true; }
    virtual uint32_t find_first() const { return 1; }
    virtual uint32_t find_next(uint32_t id) const { return id + 1; }

    inline bool iterable() const { return iterable_; }

protected:
    bool reverse_;
    bool iterable_;
};

}

NS_IZENELIB_IR_END

#endif
