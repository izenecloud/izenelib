/*
 * fujimapCommon.hpp
 * Copyright (c) 2010 Daisuke Okanohara All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FUJIMAP_COMMON_HPP__
#define FUJIMAP_COMMON_HPP__

#include <types.h>
#include <cassert>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

static const uint32_t R        = 3;                 ///< Hash Num in fujimap_block
static const uint32_t FPLEN    = 0;             ///< Default length of false positive check bits
static const uint32_t TMPN     = 1000000;        ///< Default size of temporary associative array
static const uint32_t KEYBLOCK = 128;        ///< # of minimum perfect hash function in fujimap_block
static const uint32_t NOTFOUND = 0x7FFFFFFFU; ///< Indicate that key is not found
static const uint64_t BITNUM   = 64;

enum EncodeType
{
    BINARY = 0,
    GAMMA  = 1
};

struct FujimapCommon
{
    template <class ValueType>
    static uint64_t log2(const ValueType& x);

    template <class ValueType>
    static uint64_t mask(const ValueType& x, const uint64_t& len);

    template <class ValueType>
    static uint64_t gammaLen(const ValueType& x);

    template <class ValueType>
    static uint64_t gammaEncodeBit(const uint64_t& pos, const ValueType& x);

    template <class ValueType>
    static ValueType gammaDecode(const ValueType& x);
};

template <class ValueType>
uint64_t FujimapCommon::log2(const ValueType& x)
{
    if (x == 0) return 0;
    uint64_t ret = 1;
    uint64_t max = sizeof(x) * 8;
    while (ret < max && x >> ret)
    {
        ++ret;
    }
    return ret;
}

template <class ValueType>
uint64_t FujimapCommon::mask(const ValueType& x, const uint64_t& len)
{
    return x & (((ValueType)1 << len) - 1);
}

template <class ValueType>
uint64_t FujimapCommon::gammaLen(const ValueType& x)
{
    return log2(x + 1) * 2 - 1;
}

template <class ValueType>
uint64_t FujimapCommon::gammaEncodeBit(const uint64_t& pos, const ValueType& x)
{
    ValueType tmp = x + 1;
    uint64_t flagLen = log2(tmp) - 1;
    assert(pos < 2 * log2(tmp) - 1);
    if (pos < flagLen) return 0;
    else if (pos == flagLen) return 1;
    else return (tmp >> (pos - flagLen - 1)) & 1;
}

template <class ValueType>
ValueType FujimapCommon::gammaDecode(const ValueType& code)
{
    uint64_t flagPos = 0;
    while (((code >> flagPos) & 1LLU) == 0 && flagPos < 64)
    {
        ++flagPos;
    }
    if (flagPos == 64) return NOTFOUND;
    return  mask(code >> (flagPos+1), flagPos) + ((ValueType)1 << flagPos) - 1;
}

}}

NS_IZENELIB_AM_END

#endif // COMMON_HPP__
