#include <ir/Zambezi/search/GallopSearch.hpp>
#include <ir/Zambezi/Utils.hpp>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

uint32_t gallopSearch(
        const uint32_t* block,
        bool reverse,
        uint32_t count,
        uint32_t index,
        uint32_t pivot)
{
    if (index >= count || LESS_THAN(block[count - 1], pivot, reverse))
        return INVALID_ID;

    if (GREATER_THAN_EQUAL(block[index], pivot, reverse))
        return index;

    if (block[count - 1] == pivot)
        return count - 1;

    int beginIndex = index;
    int hop = 1;
    int tempIndex = beginIndex + 1;
    while ((uint32_t)tempIndex < count && LESS_THAN_EQUAL(block[tempIndex], pivot, reverse))
    {
        beginIndex = tempIndex;
        tempIndex += hop;
        hop *= 2;
    }

    if (block[beginIndex] == pivot)
        return beginIndex;

    int endIndex = count - 1;
    hop = 1;
    tempIndex = endIndex - 1;
    while (tempIndex >= 0 && GREATER_THAN(block[tempIndex], pivot, reverse))
    {
        endIndex = tempIndex;
        tempIndex -= hop;
        hop *= 2;
    }

    if (block[endIndex] == pivot)
        return endIndex;

    // Binary search between begin and end indexes
    while (beginIndex < endIndex)
    {
        uint32_t mid = beginIndex + (endIndex - beginIndex) / 2;

        if (GREATER_THAN(block[mid], pivot, reverse))
        {
            endIndex = mid;
        }
        else if (LESS_THAN(block[mid], pivot, reverse))
        {
            beginIndex = mid + 1;
        }
        else
        {
            return mid;
        }
    }

    return endIndex;
}

}

NS_IZENELIB_IR_END
