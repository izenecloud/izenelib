#ifndef IZENELIB_IR_ZAMBEZI_SEGMENT_POOL_HPP
#define IZENELIB_IR_ZAMBEZI_SEGMENT_POOL_HPP

#include <types.h>
#include <util/compression/int/fastpfor/simdfastpfor.h>

#include <iostream>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class SegmentPool
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
    SegmentPool(
            uint32_t numberOfPools, bool reverse, bool bloomEnabled,
            bool nbHash, bool bitsPerElement);

    ~SegmentPool();

    void save(std::ostream& ostr) const;

    void load(std::istream& istr);

    /**
     * Whether or not the index contains term frequency (tf) information
     */
    bool isTermFrequencyPresent() const;

    /**
     * Whether or not the index is a positional inverted index.
     */
    bool isPositional() const;

    /**
     * Compress and write a segment into a non-positional segment pool,
     * and link it to the previous segment (if present)
     *
     * @param docid_list Document ids
     * @param len Number of document ids
     * @param tailPointer Pointer to the previous segment
     * @return Pointer to the new segment
     */
    size_t compressAndAddNonPositional(
            SIMDFastPFor& codec,
            uint32_t* docid_list,
            uint32_t len, size_t tailPointer);

    /**
     * Compress and write a segment into a non-positional segment pool with term frequencies,
     * and link it to the previous segment (if present)
     *
     * @param docid_list Document ids
     * @param tf_list Term frequencies
     * @param len Number of document ids
     * @param tailPointer Pointer to the previous segment
     * @return Pointer to the new segment
     */

    size_t compressAndAddTfOnly(
            SIMDFastPFor& codec,
            uint32_t* docid_list, uint32_t* tf_list,
            uint32_t len, size_t tailPointer);

    /**
     * Compress and write a segment into a positional segment pool,
     * and link it to the previous segment (if present)
     *
     * @param docid_list Document ids
     * @param tf_list Term frequencies
     * @param position_list List of gap-encoded term positions
     * @param len Number of document ids
     * @param plen Number of positions
     * @param tailPointer Pointer to the previous segment
     * @return Pointer to the new segment
     */
    size_t compressAndAddPositional(
            SIMDFastPFor& codec,
            uint32_t* docid_list, uint32_t* tf_list, uint32_t* position_list,
            uint32_t len, uint32_t plen, size_t tailPointer);

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
    uint32_t decompressDocidBlock(
            SIMDFastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    uint32_t decompressTfBlock(
            SIMDFastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    /**
     * Retrieved the number of positions stored in the block
     * pointed to by "pointer".
     */
    uint32_t numberOfPositionBlocks(size_t pointer) const;

    /**
     * Decompressed the position block into the "outBlock."
     * Note that outBlock's length must be:
     *
     *     numberOfPositionBlocks() * BLOCK_SIZE,
     *
     * where BLOCK_SIZE is 128.
     */
    uint32_t decompressPositionBlock(
//          SIMDFastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    void decompressPositions(
//          SIMDFastPFor& codec,
            uint32_t* tf_list, uint32_t index, size_t pointer, uint32_t* out) const;

    /**
     * If Bloom filter chains are present, perform a membership test
     *
     * @param docid Test document id
     * @param pointer Pointer to segment
     * @return Whether or not input docid exists in the Bloom filter chain
     */
    bool containsDocid(uint32_t docid, size_t& pointer) const;

    void bwandAnd(
            std::vector<size_t>& headPointers,
            uint32_t hits,
            std::vector<uint32_t>& docid_list) const;

    void bwandOr(
            std::vector<size_t>& headPointers,
            const std::vector<float>& UB,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    void wand(
            std::vector<size_t>& headPointers,
            const std::vector<uint32_t>& df,
            const std::vector<float>& UB,
            const std::vector<uint32_t>& docLen,
            uint32_t totalDocs,
            float avgDocLen,
            uint32_t hits,
            bool hasTf,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

private:
    friend class InvertedIndex;

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
