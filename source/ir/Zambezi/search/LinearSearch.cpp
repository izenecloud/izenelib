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
    if (index >= count || LESS_THAN(block[count - 1], pivot, reverse))
        return INVALID_ID;

    const __m128i* data = reinterpret_cast<const __m128i*>(&block[index]);
    const __m128i* tmp = data;
    const __m128i pivot4 = _mm_set1_epi32(pivot);

    if (reverse)
    {
        for (;; tmp += 4)
        {
            const __m128i cmp0 = _mm_cmpgt_epi32(_mm_loadu_si128(tmp), pivot4);
            const __m128i cmp1 = _mm_cmpgt_epi32(_mm_loadu_si128(tmp + 1), pivot4);
            const __m128i cmp2 = _mm_cmpgt_epi32(_mm_loadu_si128(tmp + 2), pivot4);
            const __m128i cmp3 = _mm_cmpgt_epi32(_mm_loadu_si128(tmp + 3), pivot4);

            int res = _mm_movemask_epi8(_mm_packs_epi16(
                        _mm_packs_epi32(cmp0, cmp1), _mm_packs_epi32(cmp2, cmp3)));

            if (res != 0xffff) return index + (tmp - data) * 4 + __builtin_ctz(~res);
        }
    }
    else
    {
        for (;; tmp += 4)
        {
            const __m128i cmp0 = _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(tmp));
            const __m128i cmp1 = _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(tmp + 1));
            const __m128i cmp2 = _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(tmp + 2));
            const __m128i cmp3 = _mm_cmpgt_epi32(pivot4, _mm_loadu_si128(tmp + 3));

            int res = _mm_movemask_epi8(_mm_packs_epi16(
                        _mm_packs_epi32(cmp0, cmp1), _mm_packs_epi32(cmp2, cmp3)));

            if (res != 0xffff) return index + (tmp - data) * 4 + __builtin_ctz(~res);
        }
    }

    assert(false);
    return INVALID_ID;
}

}

NS_IZENELIB_IR_END
