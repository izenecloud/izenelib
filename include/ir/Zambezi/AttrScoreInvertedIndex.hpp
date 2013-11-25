#ifndef IZENELIB_IR_ZAMBEZI_ATTR_SCORE_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_ATTR_SCORE_INVERTED_INDEX_HPP

#include "ZambeziIndex.hpp"
#include "SegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/AttrScoreBufferMaps.hpp"
#include "Utils.hpp"
#include "Consts.hpp"
#include <util/compression/int/fastpfor/fastpfor.h>

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class AttrScoreInvertedIndex : public ZambeziIndex
{
public:
    AttrScoreInvertedIndex(
            uint32_t maxPoolSize = MAX_POOL_SIZE,
            uint32_t numberOfPools = NUMBER_OF_POOLS,
            bool reverse = true);

    virtual ~AttrScoreInvertedIndex();

    virtual void save(std::ostream& ostr) const;
    virtual void load(std::istream& istr);


    /// @brief: interface to build AttrScore zambezi index;
    /// @docid: must be used; 
    /// @term_list:
    /// for example: if the title is:"aa bb cc dd aa cc";
    /// then, the @term_list is {aa, bb, cc, dd}, each @term in @term_list should be unique;
    /// @score_list: the score of its term;
    virtual void insertDoc(
            uint32_t docid,
            const std::vector<std::string>& term_list,
            const std::vector<uint32_t>& score_list);

    virtual void flush();

    virtual uint32_t totalDocNum() const;

    /// @algorithm, is not used here, just for unify interface;
    /// @term_list, each pair includes: term and its score in the query;
    /// @hits, the max number of hit number; 
    /// @search_buffer:to check if we need to search the data in buffer_map;
    
    virtual void retrievalWithBuffer(
            Algorithm algorithm,
            const std::vector<std::pair<std::string, int> >& term_list,
            uint32_t hits,
            bool search_buffer,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const
    {
        std::vector<std::pair<std::pair<uint32_t, int>, size_t> > queries;

        uint32_t minimumDf = 0xFFFFFFFF;
        bool hit_buffer = false;

        for (uint32_t i = 0; i < term_list.size(); ++i)
        {
            uint32_t termid = dictionary_.getTermId(term_list[i].first);
            if (termid != INVALID_ID)
            {
                size_t pointer = pointers_.headPointers_.get(termid);
                if (pointer != UNDEFINED_POINTER)
                {
                    queries.push_back(std::make_pair(std::make_pair(pointers_.df_.get(termid), term_list[i].second), pointer));
                    minimumDf = std::min(queries.back().first.first, minimumDf);
                }
                else if (search_buffer)
                {
                    const std::vector<uint32_t>& docBuffer = buffer_.docid_[termid];

                    if (!hit_buffer)
                    {
                        const std::vector<uint32_t>& scoreBuffer = buffer_.score_[termid];
                        docid_list.clear();
                        score_list.clear();
                        for (uint32_t j = 0; j < docBuffer.size(); ++j)
                        {
                            docid_list.push_back(docBuffer[j]);
                            score_list.push_back(scoreBuffer[j]);
                        }
                        hit_buffer = true;
                    }
                    else
                    {
                        uint32_t iSet = 0, iCurrent = 0, iBuffer = 0;

                        while (true)
                        {
                            if (docBuffer[i] == docid_list[iCurrent])
                            {
                                docid_list[iSet] = docid_list[iCurrent];
                                score_list[iSet++] = score_list[iCurrent];

                                if (++iCurrent == docid_list.size()) break;
                                if (++iBuffer == docBuffer.size()) break;
                            }
                            else if (docBuffer[iBuffer] < docid_list[iCurrent])
                            {
                                if (++iBuffer == docBuffer.size()) break;
                            }
                            else
                            {
                                if (++iCurrent == docid_list.size()) break;
                            }
                        }

                        docid_list.resize(iSet);
                        score_list.resize(iSet);
                        if (iSet == 0) return;
                    }
                }
            }
        }

        if (queries.empty()) return;

        std::sort(queries.begin(), queries.end());

        std::vector<int> qScores(queries.size());
        std::vector<size_t> qHeadPointers(queries.size());

        for (uint32_t i = 0; i < queries.size(); ++i)
        {
            qScores[i] = queries[i].first.second;
            qHeadPointers[i] = queries[i].second;
        }

        intersectSvS_(qHeadPointers, qScores, minimumDf, hits, hit_buffer, docid_list, score_list);
    }

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
            size_t lastPointer,
            size_t nextPointer);

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
            std::vector<float>& score_list) const;

    void intersectSvS_(
            std::vector<size_t>& headPointers,
            const std::vector<int>& qScores,
            uint32_t minDf,
            uint32_t hits,
            bool hit_buffer,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const
    {
        FastPFor codec;

        if (hit_buffer)
        {
            for (uint32_t i = 0; i < headPointers.size(); ++i)
            {
                if (docid_list.empty()) return;
                intersectSetPostingsList_(codec, headPointers[i], qScores[i], docid_list, score_list);
            }

            return;
        }

        if (headPointers.size() < 2)
        {
            uint32_t block[BLOCK_SIZE];
            uint32_t sblock[BLOCK_SIZE];
            uint32_t length = std::min(minDf, hits);

            docid_list.reserve(length);
            score_list.reserve(length);

            size_t t = headPointers[0];
            while (t != UNDEFINED_POINTER && docid_list.size() < length)
            {
                uint32_t c = decompressDocidBlock_(codec, &block[0], t);
                decompressScoreBlock_(codec, &sblock[0], t);

                for (uint32_t i = 0; i < c; ++i)
                {
                    docid_list.push_back(block[i]);
                    score_list.push_back(sblock[i]);
                    if (docid_list.size() == length) break;
                }
                t = pool_.nextPointer(t);
            }

            return;
        }

        docid_list.reserve(minDf);
        score_list.reserve(minDf);

        intersectPostingsLists_(codec, headPointers[0], headPointers[1], qScores[0], qScores[1], docid_list, score_list);

        for (uint32_t i = 2; i < headPointers.size(); ++i)
        {
            if (docid_list.empty()) return;
            intersectSetPostingsList_(codec, headPointers[i], qScores[i], docid_list, score_list);
        }

        if (hits < docid_list.size())
        {
            docid_list.resize(hits);
            score_list.resize(hits);
        }
    }

    bool gallopSearch_(
            FastPFor& codec,
            uint32_t* blockDocid,
            uint32_t* blockScore,
            uint32_t& count,
            uint32_t& index,
            size_t& pointer,
            uint32_t pivot) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            size_t pointer0,
            size_t pointer1,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

    void intersectPostingsLists_(
            FastPFor& codec,
            size_t pointer0,
            size_t pointer1,
            int weight0,
            int weight1,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const
    {
        uint32_t blockDocid0[BLOCK_SIZE];
        uint32_t blockDocid1[BLOCK_SIZE];
        uint32_t blockScore0[BLOCK_SIZE];
        uint32_t blockScore1[BLOCK_SIZE];

        uint32_t c0 = decompressDocidBlock_(codec, &blockDocid0[0], pointer0), i0 = 0;
        uint32_t c1 = decompressDocidBlock_(codec, &blockDocid1[0], pointer1), i1 = 0;
        decompressScoreBlock_(codec, &blockScore0[0], pointer0);
        decompressScoreBlock_(codec, &blockScore1[0], pointer1);

        while (true)
        {
            if (blockDocid0[i0] == blockDocid1[i1])
            {
                docid_list.push_back(blockDocid0[i0]);
                score_list.push_back(blockScore0[i0] * weight0 + blockScore1[i1] * weight1);
                
                if (++i0 == c0)
                {
                    if ((pointer0 = pool_.nextPointer(pointer0)) == UNDEFINED_POINTER)
                        break;

                    c0 = decompressDocidBlock_(codec, &blockDocid0[0], pointer0);
                    decompressScoreBlock_(codec, &blockScore0[0], pointer0);
                    i0 = 0;
                }
                if (++i1 == c1)
                {
                    if ((pointer1 = pool_.nextPointer(pointer1)) == UNDEFINED_POINTER)
                        break;

                    c1 = decompressDocidBlock_(codec, &blockDocid1[0], pointer1);
                    decompressScoreBlock_(codec, &blockScore1[0], pointer1);
                    i1 = 0;
                }
            }
            else if (LESS_THAN(blockDocid0[i0], blockDocid1[i1], pool_.reverse_))
            {
                if (!gallopSearch_(codec, blockDocid0, blockScore0, c0, ++i0, pointer0, blockDocid1[i1]))
                    break;
            }
            else
            {
                if (!gallopSearch_(codec, blockDocid1, blockScore1, c1, ++i1, pointer1, blockDocid0[i0]))
                    break;
            }
        }
    }

    void intersectSetPostingsList_(
            FastPFor& codec,
            size_t pointer,
            int weight,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const;

private:
    AttrScoreBufferMaps buffer_;
    SegmentPool pool_;
    Dictionary<std::string> dictionary_;
    Pointers pointers_;

    FastPFor codec_;

    static const size_t BUFFER_SIZE = 1024;
    uint32_t segment_[BUFFER_SIZE];
};

}

NS_IZENELIB_IR_END

#endif
