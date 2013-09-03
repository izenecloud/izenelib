#ifndef IZENELIB_IR_ZAMBEZI_NEW_SEGMENT_POOL_HPP
#define IZENELIB_IR_ZAMBEZI_NEW_SEGMENT_POOL_HPP

#include <types.h>
#include <util/compression/int/fastpfor/fastpfor.h>

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
     */
    NewSegmentPool(uint32_t numberOfPools, bool reverse);

    ~NewSegmentPool();

    void save(std::ostream& ostr) const;

    void load(std::istream& istr);

    inline bool isReverse() const
    {
        return reverse_;
    }

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
            FastPFor& codec,
            uint32_t* docid_list, uint32_t* tf_list,
            uint32_t len, size_t tailPointer);

    /**
     * Given the current pointer, this function returns
     * the next pointer. If the current pointer points to
     * the last block (i.e., there is no "next" block),
     * then this function returns UNDEFINED_POINTER.
     */
    size_t nextPointer(size_t pointer) const;

    size_t nextPointer(size_t pointer, uint32_t pivot) const;

    /**
     * Decompresses the docid block from the segment pointed to by "pointer,"
     * into the "outBlock" buffer. Block size is 128.
     *
     * Note that outBlock must be at least 128 integers long.
     */
    uint32_t decompressDocidBlock(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    uint32_t decompressScoreBlock(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    void wand(
            std::vector<size_t>& headPointers,
            uint32_t threshold,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    void intersectSvS(
            std::vector<size_t>& headPointers,
            uint32_t minDf,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

private:
    bool gallopSearch_(
            FastPFor& codec,
            std::vector<uint32_t>& blockDocid,
            std::vector<uint32_t>& blockScore,
            uint32_t& count, uint32_t& index, size_t& pointer,
            uint32_t pivot) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            size_t pointer0, size_t pointer1,
            uint32_t minDf,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

    void intersectSetPostingsList_(
            FastPFor& codec,
            size_t pointer,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

private:
    uint32_t numberOfPools_;
    uint32_t segment_;
    uint32_t offset_;

    // Whether or not postings are stored backwards
    bool reverse_;

    // Segment pool
    std::vector<uint32_t*> pool_;
};

}

NS_IZENELIB_IR_END

#endif
