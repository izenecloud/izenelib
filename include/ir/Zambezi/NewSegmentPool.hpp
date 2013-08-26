#ifndef IZENELIB_IR_ZAMBEZI_NEW_SEGMENT_POOL_HPP
#define IZENELIB_IR_ZAMBEZI_NEW_SEGMENT_POOL_HPP

#include <types.h>

#include <iostream>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class NewSegmentPool
{
public:
    /**
     * Create a new segment pool.
     *
     * @param numberOfPools Number of pools, where each pool is an array of integers
     * @param reverse Whether to store postings in reverse order (e.g., to index tweets)
     * @param bloomEnabled Whether or not to use Bloom filter chains (to be used with BWAND)
     * @param nbHash If Bloom filter chains are enabled,
     *        this indicates the number of hash functions
     * @param bitsPerElement If Bloom filter chains are enabled,
     *        this indicates number of bits per element
     */
    NewSegmentPool(
            uint32_t numberOfPools, bool reverse, bool bloomEnabled,
            bool nbHash, bool bitsPerElement);

    ~NewSegmentPool();

    void save(std::ostream& ostr) const;

    void load(std::istream& istr);

    /**
     * Compress and write a segment into a non-positional segment pool with term frequencies,
     * and link it to the previous segment (if present)
     *
     * @param docid_list Document ids
     * @param score_list Term score in docs
     * @param len Number of document ids
     * @param tailPointer Pointer to the previous segment
     * @return Pointer to the new segment
     */

    size_t compressAndAppend(
            uint32_t* docid_list, uint32_t* tf_list,
            uint32_t len, size_t tailPointer);

    /**
     * Given the current pointer, this function returns
     * the next pointer. If the current pointer points to
     * the last block (i.e., there is no "next" block),
     * then this function returns UNDEFINED_POINTER.
     */
    size_t nextPointer(size_t pointer) const;

    /**
     * Decompresses the docid block from the segment pointed to by "pointer,"
     * into the "outBlock" buffer. Block size is 128.
     *
     * Note that outBlock must be at least 128 integers long.
     */
    uint32_t decompressDocidBlock(uint32_t* outBlock, size_t pointer) const;

    uint32_t decompressScoreBlock(uint32_t* outBlock, size_t pointer) const;

    /**
     * If Bloom filter chains are present, perform a membership test
     *
     * @param docid Test document id
     * @param pointer Pointer to segment
     * @return Whether or not input docid exists in the Bloom filter chain
     */
    bool containsDocid(uint32_t docid, size_t& pointer) const;

private:
    friend class NewInvertedIndex;

    uint32_t numberOfPools_;
    uint32_t segment_;
    uint32_t offset_;

    // Whether or not postings are stored backwards
    bool reverse_;

    // if Bloom filters enabled
    bool bloomEnabled_;
    uint32_t nbHash_;
    uint32_t bitsPerElement_;

    // Segment pool
    std::vector<uint32_t*> pool_;
};

}

NS_IZENELIB_IR_END

#endif
