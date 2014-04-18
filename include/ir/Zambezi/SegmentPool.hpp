#ifndef IZENELIB_IR_ZAMBEZI_SEGMENT_POOL_HPP
#define IZENELIB_IR_ZAMBEZI_SEGMENT_POOL_HPP

#include <types.h>

#include <boost/shared_array.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class SegmentPool
{
public:
    SegmentPool(uint32_t maxPoolSize, uint32_t numberOfPools);

    ~SegmentPool();

    void save(std::ostream& ostr) const;

    void load(std::istream& istr);

    size_t appendSegment(
            uint32_t* dataSegment,
            uint32_t maxDocId,
            uint32_t len,
            size_t lastPointer,
            size_t nextPointer);

    size_t nextPointer(size_t pointer) const;

    size_t nextPointer(size_t pointer, uint32_t pivot, uint32_t reverse) const;

public:
    uint32_t maxPoolSize_;
    uint32_t numberOfPools_;
    uint32_t segment_;
    uint32_t offset_;

    std::vector<boost::shared_array<uint32_t> > pool_;
};

}

NS_IZENELIB_IR_END

#endif
