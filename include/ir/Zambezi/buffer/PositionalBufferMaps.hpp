#ifndef IZENELIB_IR_ZAMBEZI_BUFFER_POSITIONAL_BUFFER_MAPS_HPP
#define IZENELIB_IR_ZAMBEZI_BUFFER_POSITIONAL_BUFFER_MAPS_HPP

#include "../Consts.hpp"

#include <iostream>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

// Buffer maps
class PositionalBufferMaps
{
public:
    /**
     * Creates buffer maps.
     *
     * @param initialSize Initial capacity of buffer maps
     * @param type Type of index (non-positional, docids and tf,
     *        or positional)
     */
    PositionalBufferMaps(uint32_t initialSize, IndexType type);
    ~PositionalBufferMaps();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    /**
     * Expand buffer maps' capacities by a factor of 2
     *
     * @param newSize New capacity of buffer maps
     */
    void expand(uint32_t newSize);

    /**
     * Whether buffer maps contain a buffer for a given vocabulary term
     *
     * @param buffer Buffer maps
     * @param k Term id
     */
    bool containsKey(uint32_t k) const;

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
    IndexType type;
    // Current capacity (number of vocabulary terms)
    uint32_t capacity;

    // Docid buffer map
    std::vector<std::vector<uint32_t> > docid;
    // Term frequency buffer map
    std::vector<std::vector<uint32_t> > tf;
    // Term positions buffer map
    std::vector<std::vector<uint32_t> > position;
    // Cursor of last position block head
    std::vector<std::vector<uint32_t> > posBlockCount;
    // Table of tail pointers for vocabulary terms
    std::vector<size_t> tailPointer;
};

}

NS_IZENELIB_IR_END

#endif
