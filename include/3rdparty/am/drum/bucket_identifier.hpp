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

#include <util/izene_serialization.h>


DRUM_BEGIN_NAMESPACE

template <class key_t>
struct BucketIdentififer
{
    static std::size_t Calculate(key_t const& key, std::size_t const& num_bucket_bits)
    {
        std::size_t key_size;
        const char* key_serial;
        izenelib::util::izene_serialization<key_t> izsKey(key);
        izsKey.write_image(key_serial, key_size);

        uint64_t fp = 0;
        std::size_t min_len = std::min(num_bucket_bits / 8, key_size);
        std::size_t i = 0;

        while (i < min_len)
        {
            fp <<= 8;
            fp |= key_serial[i++];
        }
        if (num_bucket_bits > i * 8)
            fp <<= num_bucket_bits - i * 8;
        else
            fp >>= i * 8 - num_bucket_bits;

        return static_cast<std::size_t>(fp);
    }
};

template <>
struct BucketIdentififer<uint32_t>
{
    static std::size_t Calculate(uint32_t const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key >> (32 - num_bucket_bits));
    }
};

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

template <>
struct BucketIdentififer<std::string>
{
    static std::size_t Calculate(std::string const& key, std::size_t const& num_bucket_bits)
    {
        uint64_t fp = 0;
        std::size_t min_len = std::min(num_bucket_bits / 8, key.length());
        std::size_t i = 0;

        while (i < min_len)
        {
            fp <<= 8;
            fp |= key[i++];
        }
        if (num_bucket_bits > i * 8)
            fp <<= num_bucket_bits - i * 8;
        else
            fp >>= i * 8 - num_bucket_bits;

        return static_cast<std::size_t>(fp);
    }
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_BUCKET_IDENTIFIER_HPP
