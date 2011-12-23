/*****************************************************************************
 The MIT License

 Copyright (c) 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef IZENELIB_DRUM_BUCKET_IDENTIFIER_HPP
#define IZENELIB_DRUM_BUCKET_IDENTIFIER_HPP

#include "config.hpp"

#include <boost/cstdint.hpp>

#include <stdint.h>


DRUM_BEGIN_NAMESPACE

template <class key_t>
struct BucketIdentififer;

template <>
struct BucketIdentififer<uint64_t>
{
    static std::size_t Calculate(uint64_t const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key >> (64 - num_bucket_bits));
    }
};

template <>
struct BucketIdentififer<uint128_t>
{
    static std::size_t Calculate(uint128_t const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key >> (128 - num_bucket_bits));
    }
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_BUCKET_IDENTIFIER_HPP
