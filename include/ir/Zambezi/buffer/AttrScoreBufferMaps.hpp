#ifndef IZENELIB_IR_ZAMBEZI_BUFFER_ATTR_SCORE_BUFFER_MAPS_HPP
#define IZENELIB_IR_ZAMBEZI_BUFFER_ATTR_SCORE_BUFFER_MAPS_HPP

#include "../Consts.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/atomic.hpp>
#include <iostream>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

// Buffer maps
class AttrScoreBufferMaps
{
public:
    /**
     * Creates buffer maps.
     *
     * @param initialSize Initial capacity of buffer maps
     */
    AttrScoreBufferMaps(uint32_t initialSize);
    ~AttrScoreBufferMaps();

    void save(std::ostream& ostr, bool reverse) const;
    void load(std::istream& istr, bool reverse);

    /**
     * Expand buffer maps' capacities by a factor of 2
     *
     * @param newSize New capacity of buffer maps
     */
    void expand(uint32_t newSize);

    /**
     * An iterator that goes through the vocabulary terms,
     * and return the index of the next buffer whose length is more
     * than a given threshold.
     *
     * @param pos Current index
     * @param minLength Minimum length to consider
     */
    uint32_t nextIndex(uint32_t pos, uint32_t minLength) const;

    boost::shared_array<uint32_t> getBuffer(uint32_t id) const;

    void resetBuffer(uint32_t id, uint32_t new_cap, bool reverse, bool copy);

public:
    // Current capacity (number of vocabulary terms)
    uint32_t capacity;

    boost::shared_array<boost::atomic_flag> flags;
    // Docid buffer map
    std::vector<boost::shared_array<uint32_t> > buffer;
    // Table of tail pointers for vocabulary terms
    std::vector<size_t> tailPointer;
};

}

NS_IZENELIB_IR_END

#endif
