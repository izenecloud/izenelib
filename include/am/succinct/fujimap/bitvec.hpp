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
    BitVec(size_t size);

    void resize(size_t size);
    size_t bvBitSize() const;

    bool getBit(size_t pos) const;
    ValueType getBits(size_t pos, size_t len) const;
    uint64_t get64Bits(size_t pos, size_t len) const;

    void setBit(size_t pos);
    void setBits(size_t pos, size_t len, const ValueType& bits);
    void set64Bits(size_t pos, size_t len, const uint64_t& bits);

    void write(std::ofstream& ofs) const;
    void read(std::ifstream& ifs);

private:
    std::vector<uint64_t> bv_;
};

template <class ValueType>
BitVec<ValueType>::BitVec()
{
}

template <class ValueType>
BitVec<ValueType>::BitVec(size_t size)
{
    resize(size);
}

template <class ValueType>
void BitVec<ValueType>::write(std::ofstream& ofs) const
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
    return bv_.size() * BITNUM;
}

template <class ValueType>
void BitVec<ValueType>::resize(size_t size)
{
    bv_.resize((size + BITNUM - 1)/ BITNUM);
    fill(bv_.begin(), bv_.end(), 0);
}

template <class ValueType>
bool BitVec<ValueType>::getBit(size_t pos) const
{
    return (bv_[(pos / BITNUM) % bv_.size()] >> (pos % BITNUM)) & 1LLU;
}

template <class ValueType>
ValueType BitVec<ValueType>::getBits(size_t pos, size_t len) const
{
    uint64_t blockInd = pos / BITNUM;
    uint64_t blockOff = pos % BITNUM;
    if (blockOff + len < BITNUM)
    {
        return FujimapCommon::mask(bv_[blockInd] >> blockOff, len);
    }
    ValueType ret = bv_[blockInd++] >> blockOff;
    blockOff = BITNUM - blockOff;
    len -= blockOff;
    while (len >= BITNUM)
    {
        ret |= ValueType(bv_[blockInd++]) << blockOff;
        blockOff += BITNUM;
        len -= BITNUM;
    }
    if (len)
    {
        ret |= ValueType(FujimapCommon::mask(bv_[blockInd], len)) << blockOff;
    }
    return ret;
}

template <class ValueType>
uint64_t BitVec<ValueType>::get64Bits(size_t pos, size_t len) const
{
    assert(len <= BITNUM);
    uint64_t blockInd = pos / BITNUM;
    uint64_t blockOff = pos % BITNUM;
    if (blockOff + len < BITNUM)
    {
        return FujimapCommon::mask(bv_[blockInd] >> blockOff, len);
    }
    uint64_t ret = bv_[blockInd++] >> blockOff;
    blockOff = BITNUM - blockOff;
    len -= blockOff;
    if (len)
    {
        ret |= FujimapCommon::mask(bv_[blockInd], len) << blockOff;
    }
    return ret;
}

template <class ValueType>
void BitVec<ValueType>::setBit(size_t pos)
{
    bv_[(pos / BITNUM) % bv_.size()] |= 1LLU << (pos % BITNUM);
}

template <class ValueType>
void BitVec<ValueType>::setBits(size_t pos, size_t len, const ValueType& bits)
{
    uint64_t blockInd = pos / BITNUM;
    uint64_t blockOff = pos % BITNUM;
    if (blockOff + len < BITNUM)
    {
        bv_[blockInd] = (bv_[blockInd] & (~(((1LLU << len) - 1) << blockOff))) | uint64_t(bits) << blockOff;
        return;
    }
    bv_[blockInd] = FujimapCommon::mask(bv_[blockInd], blockOff);
    bv_[blockInd++] |= uint64_t(bits) << blockOff;
    blockOff = BITNUM - blockOff;
    len -= blockOff;
    while (len > BITNUM)
    {
        bv_[blockInd++] = uint64_t(bits >> blockOff);
        blockOff += BITNUM;
        len -= BITNUM;
    }
    if (len)
    {
        bv_[blockInd] = (bv_[blockInd] & (~((1LLU << len) - 1))) | uint64_t(bits >> blockOff);
    }
}

template <class ValueType>
void BitVec<ValueType>::set64Bits(size_t pos, size_t len, const uint64_t& bits)
{
    uint64_t blockInd = pos / BITNUM;
    uint64_t blockOff = pos % BITNUM;
    if (blockOff + len < BITNUM)
    {
        bv_[blockInd] = (bv_[blockInd] & (~(((1LLU << len) - 1) << blockOff))) | bits << blockOff;
        return;
    }
    bv_[blockInd] = FujimapCommon::mask(bv_[blockInd], blockOff);
    bv_[blockInd++] |= bits << blockOff;
    blockOff = BITNUM - blockOff;
    len -= blockOff;
    if (len)
    {
        bv_[blockInd] = (bv_[blockInd] & (~((1LLU << len) - 1))) | bits >> blockOff;
    }
}

}}

NS_IZENELIB_AM_END

#endif /// BITVEC_HPP__
