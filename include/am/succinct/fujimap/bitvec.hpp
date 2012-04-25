/*
 * bitVec.hpp
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

#ifndef BITVEC_HPP__
#define BITVEC_HPP__

#include <vector>
#include <fstream>
#include <types.h>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

class BitVec
{
public:
    BitVec();
    BitVec(const size_t size);

    void resize(const size_t size);
    uint64_t getBit (const size_t pos) const;
    uint64_t getBits(const size_t pos, const size_t len) const;
    //void buildSelect();
    //uint64_t select(const size_t ind) const;
    //uint64_t leftZeros(const size_t ind) const;
    void setBit(const size_t pos);
    void setBits(const size_t pos, const size_t len, const uint64_t bits);

    size_t bvSize() const;

    void write(std::ofstream& ofs);
    void read(std::ifstream& ifs);

private:
    //static uint32_t popCount(const uint32_t i);

    std::vector<uint64_t> bv_;
};

}}
NS_IZENELIB_AM_END

#endif /// BITVEC_HPP__

