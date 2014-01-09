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

    struct ElemType
    {
        uint32_t docid;
        uint32_t score;

        ElemType() : docid(), score() {}
        ElemType(uint32_t id, uint32_t sc) : docid(id), score(sc) {}

        operator uint32_t() const { return docid; }
    };

    typedef std::vector<ElemType> PostingType;

    boost::shared_ptr<PostingType> getPosting(uint32_t id) const;

    void resizePosting(uint32_t id, size_t new_size);

public:
    // Current capacity (number of vocabulary terms)
    uint32_t capacity;

    boost::shared_array<boost::atomic_flag> flags;
    // Docid buffer map
    std::vector<boost::shared_ptr<PostingType> > buffer;
    // Table of tail pointers for vocabulary terms
    std::vector<size_t> tailPointer;
};

}

NS_IZENELIB_IR_END

#endif
