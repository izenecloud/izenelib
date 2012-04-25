/*
 * bitVec.cpp
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

#include <algorithm>
#include <cassert>
#include <am/succinct/fujimap/bitvec.hpp>
#include <am/succinct/fujimap/fujimap_common.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

BitVec::BitVec()
{
}

BitVec::BitVec(const size_t size)
{
    resize(size);
}

void BitVec::write(ofstream& ofs)
{
    uint64_t bvSize = static_cast<uint64_t>(bv_.size());
    ofs.write((const char*)(&bvSize), sizeof(bvSize));
    ofs.write((const char*)(&bv_[0]), sizeof(bv_[0]) * bvSize);
}

void BitVec::read(ifstream& ifs)
{
    uint64_t bvSize = 0;
    ifs.read((char*)(&bvSize), sizeof(bvSize));
    bv_.resize(bvSize);
    ifs.read((char*)(&bv_[0]), sizeof(bv_[0]) * bvSize);
}


size_t BitVec::bvSize() const
{
    return bv_.size();
}

/*
uint64_t BitVec::popCount(const uint32_t i) {
  uint64_t r = i;
  r = ((r & 0xAAAAAAAA) >> 1) + (r & 0x55555555);
  r = ((r & 0xCCCCCCCC) >> 2) + (r & 0x33333333);
  r = ((r >> 4) + r) & 0x0F0F0F0F;
  r = (r>>8) + r;
  return ((r>>16) + r) & 0x3F;
}
*/

/*
void BitVec::buildSelect(){
  uint32_t sum = 0;
  for (size_t i = 0; i < bv.size(); ++i){
    cum.push_back(sum);
    sum += popCount(bv[i]);
  }
  cum.push_back(sum); // sentinel
}
*/

/*
uint32_t BitVec::leftZeros(const size_t ind) const {
  return select(ind) - ind;
}
*/

/*
uint32_t BitVec::select(const size_t ind) const {
  uint32_t x = ind+1;
  size_t blockPos = lower_bound(cum.begin(), cum.end(), x) - cum.begin();
  //cerr << "blockPos:" << blockPos << " " << cum.size() << " " << x << " " << cum.back() << endl;
  assert(blockPos != cum.size());
  assert(cum[blockPos-1] < x);
  assert(blockPos > 0);
  uint32_t cur = cum[blockPos-1];
  for (uint32_t pos = (blockPos-1) * 32; ;++pos){
    if (getBit(pos)){
      ++cur;
      if (cur == x) return pos;
    }
    if (pos == blockPos * 32){
      cerr << cum[blockPos-1] << " " << x << " " << cum[blockPos] << endl;
      assert(false);
    }
  }
}
*/

void BitVec::resize(const size_t size)
{
    bv_.resize((size + BITNUM -1)/ BITNUM);
    fill(bv_.begin(), bv_.end(), 0);
}

uint64_t BitVec::getBit(const size_t pos) const
{
    return (bv_[(pos/BITNUM) % bv_.size()] >> (pos%BITNUM)) & 1LLU;
}

uint64_t BitVec::getBits(const size_t pos, const size_t len) const
{
    assert(len <= BITNUM);
    uint64_t blockInd1    = pos / BITNUM;
    uint64_t blockOffset1 = pos % BITNUM;
    if (blockOffset1 + len <= BITNUM)
    {
        return mask(bv_[blockInd1] >> blockOffset1, len);
    }
    else
    {
        uint64_t blockInd2    = ((pos + len - 1) / BITNUM) % bv_.size();
        return  mask((bv_[blockInd1] >> blockOffset1) + (bv_[blockInd2] << (BITNUM - blockOffset1)), len);
    }
}

void BitVec::setBit(const size_t pos)
{
    bv_[(pos/BITNUM) % bv_.size()] |= (1LLU << (pos % BITNUM));
}

void BitVec::setBits(const size_t pos, const size_t len, const uint64_t bits)
{
    assert((pos + len - 1) / BITNUM < bv_.size());
    uint64_t blockInd1    = pos / BITNUM;
    uint64_t blockOffset1 = pos % BITNUM;
    if (blockOffset1 + len <= BITNUM)
    {
        bv_[blockInd1] = (bv_[blockInd1] & (~(((1LLU << len) - 1) << blockOffset1))) |
                         bits << blockOffset1;
    }
    else
    {
        uint64_t blockOffset2 = (pos + len) % BITNUM;
        bv_[blockInd1] = mask(bv_[blockInd1], blockOffset1) | (bits << blockOffset1);
        bv_[blockInd1+1] = (bv_[blockInd1+1] & (~((1LLU << blockOffset2) - 1))) | (bits >> (BITNUM - blockOffset1));
    }
}

}}

NS_IZENELIB_AM_END

