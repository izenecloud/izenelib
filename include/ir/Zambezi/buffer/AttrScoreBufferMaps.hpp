#ifndef IZENELIB_IR_ZAMBEZI_BUFFER_ATTR_SCORE_BUFFER_MAPS_HPP
#define IZENELIB_IR_ZAMBEZI_BUFFER_ATTR_SCORE_BUFFER_MAPS_HPP

#include "../Consts.hpp"

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

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

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
     * @param buffer Buffer map
     * @param pos Current index
     * @param minLength Minimum length to consider
     */
    uint32_t nextIndex(uint32_t pos, uint32_t minLength) const;

public:
    // Current capacity (number of vocabulary terms)
    uint32_t capacity_;

    // Docid buffer map
    std::vector<std::vector<uint32_t> > docid_;
    // Doc-term score buffer map
    std::vector<std::vector<uint32_t> > score_;
    // Table of tail pointers for vocabulary terms
    std::vector<size_t> tailPointer_;
};

}

NS_IZENELIB_IR_END

#endif
