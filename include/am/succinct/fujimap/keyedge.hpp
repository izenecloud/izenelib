/*
 * keyEdge.hpp
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

#ifndef KEYEDGE_HPP__
#define KEYEDGE_HPP__

#include "fujimap_common.hpp"

#include <string>
#include <vector>
#include <fstream>

#include <iostream>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

/*
 * Intermediated representation of key/value
 * used in HashMap.
 */

void hash(const char* str, const size_t len, const uint64_t seed,
          uint64_t& a, uint64_t& b, uint64_t& c);

template <class ValueType>
struct KeyEdge
{
    KeyEdge();
    KeyEdge(const char* str, const size_t len, const ValueType& code,
            const uint64_t seed);

    uint64_t get(uint64_t i, uint64_t bn) const
    {
        return (v[i] % bn) + bn * i;
    }

    bool operator < (const KeyEdge& k) const
    {
        for (uint64_t i = 0; i < R; ++i)
        {
            if (v[i] != k.v[i]) return v[i] < k.v[i];
        }
        return false;
    }

    void save(std::ofstream& ofs);
    void load(std::ifstream& ifs);

    uint64_t v[R];
    ValueType code;
};

template <class ValueType>
KeyEdge<ValueType>::KeyEdge(const char* str, const size_t len, const ValueType& code,
                            const uint64_t seed) : code(code)
{
    hash(str, len, seed, v[0], v[1], v[2]);
}

template <class ValueType>
KeyEdge<ValueType>::KeyEdge() : code(0)
{
}

template <class ValueType>
void KeyEdge<ValueType>::save(std::ofstream& ofs)
{
    ofs.write((const char*)(&code), sizeof(code));
    ofs.write((const char*)(&v[0]), sizeof(v[0]) * R);
}

template <class ValueType>
void KeyEdge<ValueType>::load(std::ifstream& ifs)
{
    ifs.read((char*)(&code), sizeof(code));
    ifs.read((char*)(&v[0]), sizeof(v[0]) * R);

}

}}

NS_IZENELIB_AM_END

#endif // KEYEDGE_HPP__
