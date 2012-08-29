#ifndef _FM_INDEX_UTILS_H
#define _FM_INDEX_UTILS_H

#include <types.h>

#include <vector>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

inline uint16_t bits(uint32_t n)
{
    if (!n) return 0;
    else return 32 - __builtin_clz(n);
}

inline uint16_t bits(uint64_t n)
{
    if (!n) return 0;
    else return 64 - __builtin_clzl(n);
}

inline uint32_t uint32_len(const uint32_t e, const size_t n)
{
    return (n * e + 31) / 32;
}

inline uint32_t get_field(const std::vector<uint32_t> &A, const size_t len, const size_t index)
{
    if (len == 0) return 0;
    size_t i = index * len / 32, j = index * len - 32 * i;
    uint32_t result;
    if (j + len <= 32)
    {
        result = (A[i] << (32 - j - len)) >> (32 - len);
    }
    else
    {
        result = A[i] >> j;
        result = result | (A[i + 1] << (64 - j - len)) >> (32 - len);
    }
    return result;
}

inline void set_field(std::vector<uint32_t> &A, const size_t len, const size_t index, const uint32_t x)
{
    if (len == 0) return;
    size_t i = index * len / 32, j = index * len - i * 32;
    uint32_t mask = (j + len < 32 ? ~0U << (j + len) : 0)
        | (j > 0 ? ~0U >> (32 - j) : 0);
    A[i] = (A[i] & mask) | x << j;
    if (j + len > 32)
    {
        mask = ((~0U) << (len + j - 32));
        A[i + 1] = (A[i + 1] & mask) | x >> (32 - j);
    }
}

inline uint32_t get_var_field(const std::vector<uint32_t> &A, const size_t ini, const size_t fin)
{
    if (ini == fin + 1) return 0;
    size_t i = ini / 32, j = ini - 32 * i;
    uint32_t result;
    uint32_t len = fin - ini + 1;
    if (j + len <= 32)
        result = (A[i] << (32 - j - len)) >> (32 - len);
    else
    {
        result = A[i] >> j;
        result = result | (A[i + 1] << (64 - j - len)) >> (32 - len);
    }
    return result;
}

inline void set_var_field(std::vector<uint32_t> &A, const size_t ini, const size_t fin, const uint32_t x)
{
    if (ini == fin + 1) return;
    uint32_t i = ini / 32, j = ini - i * 32;
    uint32_t len = fin - ini + 1;
    uint32_t mask = ((j + len) < 32 ? ~0U << (j + len) : 0)
        | (j > 0 ? ~0U >> (32 - j) : 0);
    A[i] = (A[i] & mask) | x << j;
    if (j + len > 32)
    {
        mask = (~0U << (len + j - 32));
        A[i + 1] = (A[i + 1] & mask) | x >> (32 - j);
    }
}

inline uint8_t popcount(const uint32_t x)
{
    return __builtin_popcount(x);
}

}
}

NS_IZENELIB_AM_END

#endif
