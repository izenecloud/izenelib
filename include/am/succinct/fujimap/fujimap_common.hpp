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

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

static const uint32_t R        = 3;                 ///< Hash Num in fujimap_block
static const uint32_t FPLEN    = 0;             ///< Default length of false positive check bits
static const uint32_t TMPN     = 1000000;        ///< Default size of temporary associative array
static const uint32_t KEYBLOCK = 128;        ///< # of minimum perfect hash function in fujimap_block
static const uint32_t NOTFOUND = 0xFFFFFFFF; ///< Indicate that key is not found
static const uint64_t BITNUM   = 64;

enum EncodeType
{
    BINARY = 0,
    GAMMA  = 1
};

/**
 * Calculate log_2(x)
 * @param x
 * @return log_2(x). log_2(0) = 0
 */
uint64_t log2(uint64_t x);
uint64_t gammaLen(uint64_t x);
uint64_t gammaEncodeBit(uint64_t pos, uint64_t x);
uint64_t gammaDecode(uint64_t x);
void printBit(const uint64_t v, const uint64_t len);
uint64_t mask(uint64_t x, uint64_t len);

}}

NS_IZENELIB_AM_END
#endif // COMMON_HPP__
