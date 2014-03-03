#ifndef IZENELIB_IR_ZAMBEZI_UTILS_HPP
#define IZENELIB_IR_ZAMBEZI_UTILS_HPP

#include "Consts.hpp"

#include <cmath>
#include <immintrin.h>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

// Operators defined based on whether or not the postings are backwards
#define LESS_THAN(X, Y, R) ((R) ? (X > Y) : (X < Y))
#define LESS_THAN_EQUAL(X, Y, R) ((R) ? (X >= Y) : (X <= Y))
#define GREATER_THAN(X, Y, R) ((R) ? (X < Y) : (X > Y))
#define GREATER_THAN_EQUAL(X, Y, R) ((R) ? (X <= Y) : (X >= Y))

// Functions to encode/decode segment and offset values
#define DECODE_SEGMENT(P) (uint32_t((P) >> 32))
#define DECODE_OFFSET(P) (uint32_t((P) & 0xFFFFFFFF))
#define ENCODE_POINTER(S, O) (size_t(S) << 32 | (O))

// Jenkin's integer hash function
inline uint32_t hash(uint32_t val, uint32_t seed)
{
    val = val + seed + (val << 12);
    val = val ^ 0xc761c23c ^ (val >> 19);
    val = val + 0x165667b1 + (val << 5);
    val = (val + 0xd3a2646c) ^ (val << 9);
    val = val + 0xfd7046c5 + (val << 3);
    return val ^ 0xb55a4f09 ^ (val >> 16);
}

inline float idf(uint32_t numDocs, uint32_t df)
{
    return logf((0.5f + numDocs - df) / (0.5f + df));
}

inline float default_bm25tf(uint32_t tf, uint32_t docLen, float avgDocLen)
{
    return (1.0f + DEFAULT_K1) * tf / (DEFAULT_K1 * (1.0f - DEFAULT_B + DEFAULT_B * docLen / avgDocLen) + tf);
}

inline float default_bm25(uint32_t tf, uint32_t df, uint32_t numDocs, uint32_t docLen, float avgDocLen)
{
    return default_bm25tf(tf, docLen, avgDocLen) * idf(numDocs, df);
}

}

NS_IZENELIB_IR_END

#endif
