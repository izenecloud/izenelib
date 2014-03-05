#ifndef IZENELIB_IR_ZAMBEZI_POSITIONAL_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_POSITIONAL_INVERTED_INDEX_HPP

#include "IndexBase.hpp"
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

class PositionalInvertedIndex : public IndexBase
{
public:
    PositionalInvertedIndex(
            IndexType type = NON_POSITIONAL,
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            uint32_t vocabSize = DEFAULT_VOCAB_SIZE,
            bool reverse = true,
            bool bloomEnabled = true,
            uint32_t nbHash = 6,
            uint32_t bitsPerElement = 16);

    virtual ~PositionalInvertedIndex();

    virtual void save(std::ostream& ostr) const;
    virtual void load(std::istream& istr);

    /// @brief: interface to build Positional zambezi index;
    /// @docid: must be used;
    /// @term_list:
    /// for example: if the title is:"aa bb cc dd aa cc";
    /// then, the term_list is {aa, bb, cc, dd, aa, cc};
    /// @score_list: not use here, just for unify insertDoc interface;
    virtual void insertDoc(uint32_t docid,
                     const std::vector<std::string>& term_list,
                     const std::vector<uint32_t>& score_list);

    virtual void flush();

    /// @algorithm, different search mode;
    /// @term_list, in pair<std::string, int>, infact olny string is used in
    /// this function;
    /// @hits, the max number of hit number;
    /// @search_buffer:is not used here, just for unify interface;
    virtual void retrieve(
            Algorithm algorithm,
            const std::vector<std::pair<std::string, int> >& term_list,
            const FilterBase* filter,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    virtual uint32_t totalDocNum() const;

private:
    void processTermBuffer_(
            std::vector<uint32_t>& docBuffer,
            std::vector<uint32_t>& tfBuffer,
            std::vector<uint32_t>& posBuffer,
            std::vector<uint32_t>& posCountBuffer,
            size_t& tailPointer,
            size_t& headPointer);

    size_t compressAndAppendBlock_(
            FastPFor& codec,
            uint32_t* docBlock,
            uint32_t* tfBlock,
            uint32_t* posBlock,
            uint32_t len,
            uint32_t tflen,
            uint32_t plen,
            size_t lastPointer,
            size_t nextPointer);

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
            const FilterBase* filter,
            uint32_t hits,
            std::vector<uint32_t>& docid_list) const;

    void bwandOr_(
            std::vector<size_t>& headPointers,
            const std::vector<float>& UB,
            const FilterBase* filter,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    void wand_(
            std::vector<size_t>& headPointers,
            const std::vector<uint32_t>& df,
            const std::vector<float>& UB,
            const std::vector<uint32_t>& docLen,
            const FilterBase* filter,
            uint32_t totalDocs,
            float avgDocLen,
            uint32_t hits,
            bool hasTf,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    void intersectSvS_(
            std::vector<size_t>& headPointers,
            const FilterBase* filter,
            uint32_t minDf,
            uint32_t hits,
            std::vector<uint32_t>& docid_list) const;

    bool iterateSegment_(
            FastPFor& codec,
            uint32_t* block,
            uint32_t& count, uint32_t& index, size_t& pointer,
            uint32_t pivot) const;

    bool gallopSearch_(
            const uint32_t* block,
            uint32_t count,
            uint32_t& index,
            uint32_t pivot) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            const FilterBase* filter,
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

    bool reverse_;

    FastPFor codec_;

    static const size_t BUFFER_SIZE = 4096;
    uint32_t segment_[BUFFER_SIZE];
};

}

NS_IZENELIB_IR_END

#endif
