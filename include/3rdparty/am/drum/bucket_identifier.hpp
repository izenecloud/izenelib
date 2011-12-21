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

template <>
struct BucketIdentififer<boost::uint64_t, 2>
{
    static std::size_t Calculate(boost::uint64_t const& key)
    {
        boost::uint64_t mask = 0x8000000000000000LL;
        boost::uint64_t bucket = mask & key;
        return static_cast<std::size_t>(bucket >> 63);
    }
};

template <>
struct BucketIdentififer<boost::uint64_t, 4>
{
    static std::size_t Calculate(boost::uint64_t const& key)
    {
        boost::uint64_t mask = 0xC000000000000000LL;
        boost::uint64_t bucket = mask & key;
        return static_cast<std::size_t>(bucket >> 62);
    }
};

template <>
struct BucketIdentififer<boost::uint64_t, 8>
{
    static std::size_t Calculate(boost::uint64_t const& key)
    {
        boost::uint64_t mask = 0xE000000000000000LL;
        boost::uint64_t bucket = mask & key;
        return static_cast<std::size_t>(bucket >> 61);
    }
};

template <>
struct BucketIdentififer<boost::uint64_t, 16>
{
    static std::size_t Calculate(boost::uint64_t const& key)
    {
        boost::uint64_t mask = 0xF000000000000000LL;
        boost::uint64_t bucket = mask & key;
        return static_cast<std::size_t>(bucket >> 60);
    }
};

template <>
struct BucketIdentififer<boost::uint64_t, 32>
{
    static std::size_t Calculate(boost::uint64_t const& key)
    {
        boost::uint64_t mask = 0xF800000000000000LL;
        boost::uint64_t bucket = mask & key;
        return static_cast<std::size_t>(bucket >> 59);
    }
};

template <>
struct BucketIdentififer<boost::uint64_t, 64>
{
    static std::size_t Calculate(boost::uint64_t const& key)
    {
        boost::uint64_t mask = 0xFC00000000000000LL;
        boost::uint64_t bucket = mask & key;
        return static_cast<std::size_t>(bucket >> 58);
    }
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_BUCKET_IDENTIFIER_HPP
