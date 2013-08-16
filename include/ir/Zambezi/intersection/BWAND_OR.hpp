#ifndef IZENELIB_IR_ZAMBEZI_INTERSECTION_BWAND_OR_HPP
#define IZENELIB_IR_ZAMBEZI_INTERSECTION_BWAND_OR_HPP

#include "../SegmentPool.hpp"

#include <vector>
#include <algorithm>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{
namespace Intersection
{

static void bwandOr(
        const SegmentPool& pool,
        const std::vector<size_t>& headPointers,
        const std::vector<float>& UB,
        uint32_t hits,
        std::vector<uint32_t>& docid_list,
        std::vector<float>& score_list)
{
    docid_list.reserve(hits);
    score_list.reserve(hits);

    std::vector<size_t> pointers(headPointers);
    std::vector<std::pair<float, uint32_t> > result_list;
    result_list.reserve(hits);
    std::greater<std::pair<float, uint32_t> > comparator;

    float threshold = .0f;
    float sumOfUB = .0f;
    for (uint32_t i = 0; i < UB.size(); ++i)
    {
        sumOfUB += UB[i];
    }

    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);
    while (pointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = pool.decompressDocidBlock(&docid_block[0], pointers[0]);

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t pivot = docid_block[i];
            float score = UB[0];
            for (uint32_t j = 1; j < pointers.size(); ++j)
            {
                if (pool.containsDocid(pivot, pointers[j]))
                {
                    score += UB[j];
                }
            }

            if (result_list.size() < hits)
            {
                result_list.push_back(std::make_pair(score, pivot));
                std::push_heap(result_list.begin(), result_list.end(), comparator);
                if (result_list.size() == hits && (threshold = result_list[0].first) == sumOfUB)
                    break;
            }
            else if (score > threshold)
            {
                std::pop_heap(result_list.begin(), result_list.end(), comparator);
                result_list.back() = std::make_pair(score, pivot);
                std::push_heap(result_list.begin(), result_list.end(), comparator);
                if ((threshold = result_list[0].first) == sumOfUB)
                    break;
            }
        }

        if (result_list.size() == hits && threshold == sumOfUB)
            break;

        pointers[0] = pool.nextPointer(pointers[0]);
    }

    for (uint32_t i = 0; i < result_list.size(); ++i)
    {
        score_list.push_back(result_list[i].first);
        docid_list.push_back(result_list[i].second);
    }
}

}
}

NS_IZENELIB_IR_END

#endif
