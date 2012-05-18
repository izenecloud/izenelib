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

#include "fujimap_common.hpp"

#include <vector>
#include <fstream>
#include <types.h>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

template <class ValueType>
class BitVec
{
public:
    BitVec();
    BitVec(const size_t size);

    void resize(const size_t size);
    bool getBit(const size_t pos) const;
    ValueType getBits(const size_t pos, const size_t len) const;
    void setBit(const size_t pos);
    void setBits(const size_t pos, const size_t len, const ValueType& bits);

    size_t bvBitSize() const;

    void write(std::ofstream& ofs);
    void read(std::ifstream& ifs);

private:
    std::vector<ValueType> bv_;
    static const uint64_t bit_num_ = sizeof(ValueType) * 8;
};

template <class ValueType>
BitVec<ValueType>::BitVec()
{
}

template <class ValueType>
BitVec<ValueType>::BitVec(const size_t size)
{
    resize(size);
}

template <class ValueType>
void BitVec<ValueType>::write(std::ofstream& ofs)
{
    uint64_t bvSize = static_cast<uint64_t>(bv_.size());
    ofs.write((const char*)(&bvSize), sizeof(bvSize));
    ofs.write((const char*)(&bv_[0]), sizeof(bv_[0]) * bvSize);
}

template <class ValueType>
void BitVec<ValueType>::read(std::ifstream& ifs)
{
    uint64_t bvSize = 0;
    ifs.read((char*)(&bvSize), sizeof(bvSize));
    bv_.resize(bvSize);
    ifs.read((char*)(&bv_[0]), sizeof(bv_[0]) * bvSize);
}


template <class ValueType>
size_t BitVec<ValueType>::bvBitSize() const
{
    return bv_.size() * bit_num_;
}

template <class ValueType>
void BitVec<ValueType>::resize(const size_t size)
{
    bv_.resize((size + bit_num_ -1)/ bit_num_);
    fill(bv_.begin(), bv_.end(), 0);
}

template <class ValueType>
bool BitVec<ValueType>::getBit(const size_t pos) const
{
    return (bv_[(pos / bit_num_) % bv_.size()] >> (pos % bit_num_)) & 1;
}

template <class ValueType>
ValueType BitVec<ValueType>::getBits(const size_t pos, const size_t len) const
{
    assert(len <= bit_num_);
    uint64_t blockInd1    = pos / bit_num_;
    uint64_t blockOffset1 = pos % bit_num_;
    if (blockOffset1 + len <= bit_num_)
    {
        return FujimapCommon::mask(bv_[blockInd1] >> blockOffset1, len);
    }
    else
    {
        uint64_t blockInd2 = ((pos + len - 1) / bit_num_) % bv_.size();
        return  FujimapCommon::mask((bv_[blockInd1] >> blockOffset1) + (bv_[blockInd2] << (bit_num_ - blockOffset1)), len);
    }
}

template <class ValueType>
void BitVec<ValueType>::setBit(const size_t pos)
{
    bv_[(pos / bit_num_) % bv_.size()] |= (ValueType)1 << (pos % bit_num_);
}

template <class ValueType>
void BitVec<ValueType>::setBits(const size_t pos, const size_t len, const ValueType& bits)
{
    assert((pos + len - 1) / bit_num_ < bv_.size());
    uint64_t blockInd1    = pos / bit_num_;
    uint64_t blockOffset1 = pos % bit_num_;
    if (blockOffset1 + len <= bit_num_)
    {
        bv_[blockInd1] = (bv_[blockInd1] & (~((((ValueType)1 << len) - 1) << blockOffset1))) |
                         bits << blockOffset1;
    }
    else
    {
        uint64_t blockOffset2 = (pos + len) % bit_num_;
        bv_[blockInd1] = FujimapCommon::mask(bv_[blockInd1], blockOffset1) | (bits << blockOffset1);
        bv_[blockInd1+1] = (bv_[blockInd1 + 1] & (~(((ValueType)1 << blockOffset2) - 1))) | (bits >> (bit_num_ - blockOffset1));
    }
}

}}

NS_IZENELIB_AM_END

#endif /// BITVEC_HPP__
