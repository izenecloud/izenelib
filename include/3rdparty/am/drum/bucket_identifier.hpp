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
#include "rabin_fingerprint.hpp"

#include <util/izene_serialization.h>


DRUM_BEGIN_NAMESPACE

template <class key_t>
struct BucketIdentifier
{
    static std::size_t Calculate(key_t const& key, std::size_t const& num_bucket_bits)
    {
        static RabinFingerprint rabin_fingerprint;

        std::size_t key_size;
        char* key_serial;
        izenelib::util::izene_serialization<key_t> izsKey(key);
        izsKey.write_image(key_serial, key_size);
        uint64_t fp = rabin_fingerprint.Compute(key_serial, key_size);

        return static_cast<std::size_t>(fp >> (64 - num_bucket_bits));
    }
};

template <>
struct BucketIdentifier<uint32_t>
{
    static std::size_t Calculate(uint32_t const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key >> (32 - num_bucket_bits));
    }
};

template <>
struct BucketIdentifier<uint64_t>
{
    static std::size_t Calculate(uint64_t const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key >> (64 - num_bucket_bits));
    }
};

template <>
struct BucketIdentifier<uint128_t>
{
    static std::size_t Calculate(uint128_t const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key >> (128 - num_bucket_bits));
    }
};

template <>
struct BucketIdentifier<std::string>
{
    static std::size_t Calculate(std::string const& key, std::size_t const& num_bucket_bits)
    {
        static RabinFingerprint rabin_fingerprint;

        uint64_t fp = rabin_fingerprint.Compute(key.data(), key.size());

        return static_cast<std::size_t>(fp >> (64 - num_bucket_bits));
    }
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_BUCKET_IDENTIFIER_HPP
