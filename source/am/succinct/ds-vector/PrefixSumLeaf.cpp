/*
 *  Copyright (c) 2012 Daisuke Okanohara
  *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include <cassert>
#include <iostream>
#include <am/succinct/ds-vector/PrefixSumLeaf.hpp>
#include <am/succinct/ds-vector/Util.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{


PrefixSumLeaf::PrefixSumLeaf() : num_(0)
{
}

PrefixSumLeaf::~PrefixSumLeaf()
{
}

void PrefixSumLeaf::Init(uint64_t num)
{
    num_ = num;
}

void PrefixSumLeaf::Clear()
{
    bit_arrays_.clear();
    num_ = 0;
}

void PrefixSumLeaf::Insert(uint64_t ind, uint64_t val)
{
    assert(ind < 64);
    assert(num_ < 64);
    uint64_t blen = Util::GetBinaryLen(val);
    if (bit_arrays_.size() < blen)
    {
        bit_arrays_.resize(blen);
    }

    uint64_t mask = (1LLU << ind) - 1;
    uint64_t not_mask = ~mask;
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        uint64_t bits = bit_arrays_[shift];
        bit_arrays_[shift] = (bits & mask) | ((bits & not_mask) << 1) | Util::GetBit(val, shift) << ind;
    }
    ++num_;
}

void PrefixSumLeaf::Increment(uint64_t ind, uint64_t val)
{
    const uint64_t set_bit = (1LLU << ind);
    uint64_t carry_bit = 0;
    for (uint64_t shift = 0;; ++shift)
    {
        if ((val >> shift) == 0 && carry_bit == 0) return;
        if (shift == bit_arrays_.size())
        {
            bit_arrays_.push_back(0);
        }
        uint64_t val_bit = (val >> shift) & 1LLU;
        uint64_t cur_bit = Util::GetBit(bit_arrays_[shift], ind);
        uint64_t sum = cur_bit + carry_bit + val_bit;

        if (cur_bit == 0 && (sum & 1LLU) == 1)
        {
            bit_arrays_[shift] |= set_bit;       // 0 -> 1
        }
        else if (cur_bit == 1 && (sum & 1LLU) == 0)
        {
            bit_arrays_[shift] &= ~set_bit;      // 1 -> 0
        }
        carry_bit = sum >> 1;
    }
    assert(false);
}

void PrefixSumLeaf::Decrement(uint64_t ind, uint64_t val)
{
    const uint64_t set_bit = (1LLU << ind);
    uint64_t carry_bit = 0;
    val = ~val + 1;
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        uint64_t val_bit = (val >> shift) & 1LLU;
        uint64_t cur_bit = Util::GetBit(bit_arrays_[shift], ind);
        uint64_t sum = cur_bit + carry_bit + val_bit;

        if (cur_bit == 0 && (sum & 1LLU) == 1)
        {
            bit_arrays_[shift] |= set_bit;       // 0 -> 1
        }
        else if (cur_bit == 1 && (sum & 1LLU) == 0)
        {
            bit_arrays_[shift] &= ~set_bit;      // 1 -> 0
        }
        carry_bit = sum >> 1;
    }
}


void PrefixSumLeaf::Set(uint64_t ind, uint64_t val)
{
    uint64_t blen = Util::GetBinaryLen(val);
    if (bit_arrays_.size() < blen)
    {
        bit_arrays_.resize(blen);
    }
    uint64_t unset_bit = ~(1LLU << ind);
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        bit_arrays_[shift] &= unset_bit;
    }
    for (uint64_t shift = 0; shift < blen; ++shift)
    {
        bit_arrays_[shift] |= (Util::GetBit(val, shift) << ind);
    }
}

uint64_t PrefixSumLeaf::Get(uint64_t ind) const
{
    uint64_t ret = 0;
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        ret += Util::GetBit(bit_arrays_[shift], ind) << shift;
    }
    return ret;
}

uint64_t PrefixSumLeaf::GetWithMask(uint64_t mask) const
{
    uint64_t ret = 0;
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        ret += Util::PopCount(bit_arrays_[shift] & mask) << shift;
    }
    return ret;
}

uint64_t PrefixSumLeaf::GetPrefixSum(uint64_t ind) const
{
    return GetWithMask((ind == 64) ? (0xFFFFFFFFFFFFFFFFLLU) : (1LLU << ind) - 1);
}

uint64_t PrefixSumLeaf::Find(uint64_t val) const
{
    if (bit_arrays_.size() == 0) return 0;

    static uint64_t masks[5];
    masks[0] = 0x5555555555555555LLU;
    masks[1] = 0x3333333333333333LLU;
    masks[2] = 0x0f0f0f0f0f0f0f0fLLU;
    masks[3] = 0x00ff00ff00ff00ffLLU;
    masks[4] = 0x0000ffff0000ffffLLU;

    uint64_t cums[64][6];
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        cums[shift][0] = bit_arrays_[shift];
    }

    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        uint64_t* cs = cums[shift];
        for (uint64_t i = 0, offset =1; i < 5; ++i, offset <<= 1)
        {
            cs[i+1] = (cs[i] & masks[i]) + ((cs[i] >> offset) & masks[i]);
        }
    }

    uint64_t ind = 0;
    uint64_t sum = 0;
    for (uint64_t width = 6; width > 0; )
    {
        --width;
        uint64_t psum = 0;
        for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
        {
            psum += Util::GetBits(cums[shift][width], ind, 1LLU << width) << shift;
        }
        if (sum + psum < val)
        {
            sum += psum;
            ind += (1LLU << width);
        }
    }
    if (sum + Get(ind) <= val)
    {
        ++ind;
    }
    return ind;
}

void PrefixSumLeaf::Print() const
{
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        Util::PrintBit(bit_arrays_[shift]);
    }
}


uint64_t PrefixSumLeaf::Split(PrefixSumLeaf& ps)
{
    uint64_t half_num = num_ / 2;
    ps.bit_arrays_.resize(bit_arrays_.size());
    uint64_t mask = (1LLU << half_num) - 1;
    for (uint64_t shift = 0; shift < bit_arrays_.size(); ++shift)
    {
        ps.bit_arrays_[shift] = bit_arrays_[shift] >> half_num;
        bit_arrays_[shift] &= mask;
    }
    ps.num_ = num_ - half_num;
    num_ = half_num;
    return half_num;
}


}
}

NS_IZENELIB_AM_END

