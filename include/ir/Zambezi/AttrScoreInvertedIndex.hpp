#ifndef IZENELIB_IR_ZAMBEZI_ATTR_SCORE_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_ATTR_SCORE_INVERTED_INDEX_HPP

#include "SegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/AttrScoreBufferMaps.hpp"
#include "Consts.hpp"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class AttrScoreInvertedIndex
{
public:
    AttrScoreInvertedIndex(
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            bool reverse = true);

    ~AttrScoreInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    void insertDoc(
            uint32_t docid,
            const std::vector<std::string>& term_list,
            const std::vector<uint32_t>& score_list);

    void flush();

    uint32_t totalDocNum() const;

    void retrieval(
            Algorithm algorithm,
            const std::vector<std::string>& term_list,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

    void retrievalAndFiltering(
            Algorithm algorithm,
            const std::vector<std::string>& term_list,
            const boost::function<bool(uint32_t)>& filter,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

private:
    void processTermBuffer_(
            std::vector<uint32_t>& docBuffer,
            std::vector<uint32_t>& scoreBuffer,
            size_t& tailPointer,
            size_t& headPointer);

    size_t compressAndAppendBlock_(
            uint32_t* docBlock,
            uint32_t* scoreBlock,
            uint32_t len,
            size_t tailPointer);

    uint32_t decompressDocidBlock_(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    uint32_t decompressScoreBlock_(
            FastPFor& codec,
            uint32_t* outBlock, size_t pointer) const;

    void intersectSvS_(
            std::vector<size_t>& headPointers,
            uint32_t minDf,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

    void intersectSvS_(
            std::vector<size_t>& headPointers,
            const boost::function<bool(uint32_t)>& filter,
            uint32_t minDf,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

    bool gallopSearch_(
            FastPFor& codec,
            std::vector<uint32_t>& blockDocid,
            std::vector<uint32_t>& blockScore,
            uint32_t& count,
            uint32_t& index,
            size_t& pointer,
            uint32_t pivot) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            size_t pointer0,
            size_t pointer1,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            size_t pointer0,
            size_t pointer1,
            const boost::function<bool(uint32_t)>& filter,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

    void intersectSetPostingsList_(
            FastPFor& codec,
            size_t pointer,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

private:
    AttrScoreBufferMaps buffer_;
    SegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    FastPFor codec_;
};

}

NS_IZENELIB_IR_END

#endif
