#ifndef IZENELIB_IR_ZAMBEZI_ATTR_SCORE_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_ATTR_SCORE_INVERTED_INDEX_HPP

#include "IndexBase.hpp"
#include "SegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/AttrScoreBufferMaps.hpp"
#include "Consts.hpp"

#include <iostream>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class AttrScoreInvertedIndex : public IndexBase
{
public:
    AttrScoreInvertedIndex(
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            uint32_t vocabSize = DEFAULT_VOCAB_SIZE,
            bool reverse = true);

    ~AttrScoreInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    /// @brief: interface to build AttrScore zambezi index;
    /// @docid: must be used;
    /// @term_list:
    /// for example: if the title is:"aa bb cc dd aa cc";
    /// then, the @term_list is {aa, bb, cc, dd}, each @term in @term_list should be unique;
    /// @score_list: the score of its term;
    void insertDoc(
            uint32_t docid,
            const std::vector<std::string>& term_list,
            const std::vector<uint32_t>& score_list);

    void flush();

    uint32_t totalDocNum() const;

    void retrieve(
            Algorithm algorithm,
            const std::vector<std::pair<std::string, int> >& term_list,
            const FilterBase* filter,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

private:
    void processTermBuffer_(
            boost::shared_array<uint32_t>& posting,
            size_t& tailPointer,
            size_t& headPointer);

    size_t compressAndAppendBlock_(
            uint32_t* docBlock,
            uint32_t* scoreBlock,
            uint32_t len,
            size_t lastPointer,
            size_t nextPointer);

    uint32_t decompressDocidBlock_(
            uint32_t* outBlock, size_t pointer) const;

    uint32_t decompressScoreBlock_(
            uint32_t* outBlock, size_t pointer) const;

    void intersectSvS_(
            const std::vector<uint32_t>& qTerms,
            const std::vector<int>& qScores,
            const FilterBase* filter,
            uint32_t minDf,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    bool unionIterate_(
            bool& in_buffer,
            const uint32_t* buffer,
            uint32_t* docid_seg,
            uint32_t* score_seg,
            uint32_t pivot,
            uint32_t& count,
            uint32_t& tail,
            uint32_t& index,
            size_t& pointer,
            uint32_t& docid,
            uint32_t& score) const;

    void intersectPostingsLists_(
            const FilterBase* filter,
            uint32_t term0,
            uint32_t term1,
            int weight0,
            int weight1,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list,
            uint32_t hits) const;

    void intersectSetPostingsList_(
            uint32_t term,
            int weight,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

private:
    AttrScoreBufferMaps buffer_;
    SegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    bool reverse_;

    static const size_t BUFFER_SIZE = 4096;
    uint32_t segment_[BUFFER_SIZE] __attribute__((aligned(16)));
};

}

NS_IZENELIB_IR_END

#endif
