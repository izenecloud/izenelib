#ifndef IZENELIB_UTIL_SKETCH_ICARDINALITY_H_
#define IZENELIB_UTIL_SKETCH_ICARDINALITY_H_

#include <common/type_defs.h>

NS_IZENELIB_UTIL_BEGIN

template <typename ElemType>
class ICardinality
{
public:
    typedef ElemType DataTypeT;
    typedef ICardinality<DataTypeT> SelfType;
    virtual size_t size() const = 0;

    virtual void updateSketch(const DataTypeT& data) = 0;
    virtual size_t getCardinate() const = 0;
    virtual void unionSketch(const SelfType* src) = 0;
    virtual ~ICardinality(){}
};

NS_IZENELIB_UTIL_END

#endif
