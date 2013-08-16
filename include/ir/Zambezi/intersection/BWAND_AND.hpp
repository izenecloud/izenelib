#ifndef IZENELIB_IR_ZAMBEZI_INTERSECTION_BWAND_AND_HPP
#define IZENELIB_IR_ZAMBEZI_INTERSECTION_BWAND_AND_HPP

#include "../SegmentPool.hpp"

#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{
namespace Intersection
{

static void bwandAnd(
        const SegmentPool& pool,
        const std::vector<size_t>& headPointers,
        uint32_t hits,
        std::vector<uint32_t>& docid_list)
{
    docid_list.reserve(hits);

    std::vector<size_t> pointers(headPointers);
    std::vector<uint32_t> docid_block(2 * BLOCK_SIZE);
    while (pointers[0] != UNDEFINED_POINTER)
    {
        uint32_t count = pool.decompressDocidBlock(&docid_block[0], pointers[0]);

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t pivot = docid_block[i];
            bool found = true;
            for (uint32_t j = 1; j < pointers.size(); ++j)
            {
                if (pointers[j] == UNDEFINED_POINTER)
                    return;

                if (!pool.containsDocid(pivot, pointers[j]))
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                docid_list.push_back(pivot);
                if (docid_list.size() >= hits)
                    return;
            }
        }

        pointers[0] = pool.nextPointer(pointers[0]);
    }
}

}
}

NS_IZENELIB_IR_END

#endif
