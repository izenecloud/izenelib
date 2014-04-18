#include <ir/Zambezi/search/LinearSearch.hpp>
#include <ir/Zambezi/Utils.hpp>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

uint32_t linearSearch(
        const uint32_t* block,
        bool reverse,
        uint32_t count,
        uint32_t index,
        uint32_t pivot)
{
//  if (index >= count || LESS_THAN(block[count - 1], pivot, reverse))
//      return INVALID_ID;

    __m128i pivot4 = _mm_set1_epi32(pivot);
    const uint32_t* tmp = block + index;

    if (reverse)
    {
        for (;; tmp += 16)
        {
            int res = _mm_movemask_epi8(_mm_packs_epi16(
                        _mm_packs_epi32(
                            _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp)), pivot4),
                            _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp) + 1), pivot4)),
                        _mm_packs_epi32(
                            _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp) + 2), pivot4),
                            _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp) + 3), pivot4))));

            if (res != 0xffff) return tmp - block + __builtin_ctz(~res);
        }
    }
    else
    {
        for (;; tmp += 16)
        {
            int res = _mm_movemask_epi8(_mm_packs_epi16(
                        _mm_packs_epi32(
                            _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp))),
                            _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp) + 1))),
                        _mm_packs_epi32(
                            _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp) + 2)),
                            _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp) + 3)))));

            if (res != 0xffff) return tmp - block + __builtin_ctz(~res);
        }
    }

    assert(false);
    return INVALID_ID;
}

}

NS_IZENELIB_IR_END
