/*
 *  Copyright (c) 2012 Daisuke Okanohara
  *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#ifndef PREFIX_SUM_UTIL_HPP_
#define PREFIX_SUM_UTIL_HPP_

#include <types.h>
#include <iostream>

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

class Util
{
public:
    inline static uint64_t GetBit(uint64_t x, uint64_t pos);
    inline static uint64_t GetBits(uint64_t x, uint64_t pos, uint64_t width);
    inline static uint64_t PopCount(uint64_t x);
    inline static uint64_t Num(uint64_t one_num, uint64_t total, uint64_t bit);
    inline static void Insert(uint64_t& x, uint64_t pos, uint64_t bit);
    inline static uint64_t GetBinaryLen(uint64_t x);
    inline static void PrintBit(uint64_t x);
};

uint64_t Util::GetBit(uint64_t x, uint64_t pos)
{
    return (x >> pos) & 1LLU;
}

uint64_t Util::GetBits(uint64_t x, uint64_t pos, uint64_t width)
{
    return (x >> pos) & ((1LLU << width) - 1);
}

uint64_t Util::PopCount(uint64_t x)
{
    x = x - ((x & 0xAAAAAAAAAAAAAAAALLU) >> 1);
    x = (x & 0x3333333333333333LLU) + ((x >> 2) & 0x3333333333333333LLU);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FLLU;
    return x * 0x0101010101010101LLU >> 56;
}

uint64_t Util::Num(uint64_t one_num, uint64_t total, uint64_t bit)
{
    if (bit) return one_num;
    else return total - one_num;
}

void Util::Insert(uint64_t& x, uint64_t pos, uint64_t bit)
{
    uint64_t mask = (1LLU << pos) - 1;
    uint64_t not_mask = ~mask;
    x = (x & mask) | ((x & not_mask) << 1) | bit << pos;
}

void Util::PrintBit(uint64_t x)
{
    for (uint64_t i = 0; i < 64; ++i)
    {
        std::cout << GetBit(x, i);
    }
    std::cout << std::endl;
}

uint64_t Util::GetBinaryLen(uint64_t x)
{
    uint64_t blen = 0;
    while (x >> blen)
    {
        ++blen;
    }
    return blen;
}

}
}

NS_IZENELIB_AM_END

#endif // PREFIX_SUM_UTIL_HPP_
