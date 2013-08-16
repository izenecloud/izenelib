#ifndef IZENELIB_IR_ZAMBEZI_BUFFER_BUFFER_MAPS_HPP
#define IZENELIB_IR_ZAMBEZI_BUFFER_BUFFER_MAPS_HPP

#include "../Consts.hpp"

#include <iostream>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

// Buffer maps
class BufferMaps
{
public:
    /**
     * Creates buffer maps.
     *
     * @param initialSize Initial capacity of buffer maps
     * @param type Type of index (non-positional, docids and tf,
     *        or positional)
     */
    BufferMaps(uint32_t initialSize, IndexType type);
    ~BufferMaps();

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

    std::vector<uint32_t>& getDocidList(uint32_t k);
    const std::vector<uint32_t>& getDocidList(uint32_t k) const;
    std::vector<uint32_t>& getTfList(uint32_t k);
    const std::vector<uint32_t>& getTfList(uint32_t k) const;
    std::vector<uint32_t>& getPositionList(uint32_t k);
    const std::vector<uint32_t>& getPositionList(uint32_t k) const;

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

private:
    friend class InvertedIndex;

    IndexType type_;
    // Current capacity (number of vocabulary terms)
    uint32_t capacity_;

    // Docid buffer map
    std::vector<std::vector<uint32_t> > docid_;
    // Term frequency buffer map
    std::vector<std::vector<uint32_t> > tf_;
    // Term positions buffer map
    std::vector<std::vector<uint32_t> > position_;
    // Table of tail pointers for vocabulary terms
    std::vector<size_t> tailPointer_;
    // Cursor of last position block head
    std::vector<uint32_t> posBlockHead_;
};

}

NS_IZENELIB_IR_END

#endif
