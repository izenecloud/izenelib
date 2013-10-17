#ifndef IZENELIB_IR_ZAMBEZI_POSITIONAL_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_POSITIONAL_INVERTED_INDEX_HPP

#include "SegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/PositionalBufferMaps.hpp"
#include "Consts.hpp"
#include <util/compression/int/fastpfor/fastpfor.h>

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class PositionalInvertedIndex
{
public:
    PositionalInvertedIndex(
            IndexType type = NON_POSITIONAL,
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            bool reverse = true,
            bool bloomEnabled = true,
            uint32_t nbHash = 3,
            uint32_t bitsPerElement = 8);

    ~PositionalInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    void insertDoc(uint32_t docid, const std::vector<std::string>& term_list);
    void flush();

    void retrieval(
            Algorithm algorithm,
            const std::vector<std::string>& term_list,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

private:
    size_t compressAndAddNonPositional_(
            FastPFor& codec,
            uint32_t* docid_list,
            uint32_t len, size_t tailPointer);

    size_t compressAndAddTfOnly_(
            FastPFor& codec,
            uint32_t* docid_list, uint32_t* tf_list,
            uint32_t len, size_t tailPointer);

    size_t compressAndAddPositional_(
            FastPFor& codec,
            uint32_t* docid_list, uint32_t* tf_list, uint32_t* position_list,
            uint32_t len, uint32_t plen, size_t tailPointer);

    uint32_t decompressDocidBlock_(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    uint32_t decompressTfBlock_(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    uint32_t numberOfPositionBlocks_(size_t pointer) const;

    uint32_t decompressPositionBlock_(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    void decompressPositions_(
            FastPFor& codec,
            uint32_t* tf_list, uint32_t index, size_t pointer, uint32_t* out) const;

    bool containsDocid_(uint32_t docid, size_t& pointer) const;

    void bwandAnd_(
            std::vector<size_t>& headPointers,
            uint32_t hits,
            std::vector<uint32_t>& docid_list) const;

    void bwandOr_(
            std::vector<size_t>& headPointers,
            const std::vector<float>& UB,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    void wand_(
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

    void intersectSvS_(
            std::vector<size_t>& headPointers,
            uint32_t minDf,
            uint32_t hits,
            std::vector<uint32_t>& docid_list) const;

    bool gallopSearch_(
            FastPFor& codec,
            std::vector<uint32_t>& blockDocid,
            uint32_t& count, uint32_t& index, size_t& pointer,
            uint32_t pivot) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            size_t pointer0, size_t pointer1,
            std::vector<uint32_t>& docid_list) const;

    void intersectSetPostingsList_(
            FastPFor& codec,
            size_t pointer,
            std::vector<uint32_t>& docid_list) const;

private:
    IndexType type_;
    PositionalBufferMaps buffer_;
    SegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    // if Bloom filters enabled
    bool bloomEnabled_;
    uint32_t nbHash_;
    uint32_t bitsPerElement_;

    FastPFor codec_;
    uint32_t segment_[4096];
};

}

NS_IZENELIB_IR_END

#endif
