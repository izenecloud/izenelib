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

    inline std::vector<uint32_t>& getDocidList(uint32_t k)
    {
        expand(k + 1);
        return docid_[k];
    }

    inline const std::vector<uint32_t>& getDocidList(uint32_t k) const
    {
        return docid_[k];
    }

    inline std::vector<uint32_t>& getTfList(uint32_t k)
    {
        if (type_ != NON_POSITIONAL)
        {
            expand(k + 1);
            return tf_[k];
        }
        return emptyList_;
    }

    inline const std::vector<uint32_t>& getTfList(uint32_t k) const
    {
        if (type_ != NON_POSITIONAL)
        {
            return tf_[k];
        }
        return emptyList_;
    }

    inline std::vector<uint32_t>& getPositionList(uint32_t k)
    {
        if (type_ == POSITIONAL)
        {
            expand(k + 1);
            return position_[k];
        }
        return emptyList_;
    }

    inline const std::vector<uint32_t>& getPositionList(uint32_t k) const
    {
        if (type_ == POSITIONAL)
        {
            return position_[k];
        }
        return emptyList_;
    }

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
    friend class PositionalInvertedIndex;

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

    std::vector<uint32_t> emptyList_;
};

}

NS_IZENELIB_IR_END

#endif
