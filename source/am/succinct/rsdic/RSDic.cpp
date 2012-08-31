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

#include <am/succinct/rsdic/Util.hpp>
#include <am/succinct/rsdic/EnumCoder.hpp>
#include <am/succinct/rsdic/RSDic.hpp>

using namespace std;


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace rsdic
{

RSDic::RSDic() : num_(), one_num_()
{
}

RSDic::~RSDic()
{
}

void RSDic::Build(const vector<uint64_t>& bv, uint64_t len)
{
    rank_blocks_.push_back(one_num_);
    pointer_blocks_.push_back(offset_);

    uint64_t index = len / kSmallBlockSize;
    uint64_t offset = len % kSmallBlockSize;

    for (uint64_t i = 0; i < index; ++i)
    {
        BuildBlock_(bv[i], kSmallBlockSize);
    }
    if (offset > 0)
    {
        BuildBlock_(bv[index], offset);
    }
}

void RSDic::BuildBlock_(uint64_t block, uint64_t bits)
{
    block = block & ((1LLU << bits) - 1);

    uint64_t prev_one_num = one_num_;
    uint64_t block_one_num = Util::PopCount(block);

    if (one_num_ % kSelectBlockSize + block_one_num > kSelectBlockSize)
    {
        select_one_inds_.push_back(num_ / kLargeBlockSize);
    }
    if ((num_ - one_num_) % kSelectBlockSize + bits - block_one_num > kSelectBlockSize)
    {
        select_zero_inds_.push_back(num_ / kLargeBlockSize);
    }
    num_ += bits;
    one_num_ += block_one_num;

    uint64_t rank_sb = one_num_ - prev_one_num;
    rank_small_blocks_.push_back(rank_sb);

    uint64_t len = EnumCoder::Len(rank_sb);
    uint64_t code;
    if (len == kSmallBlockSize)
        code = block; // use raw
    else
        code = EnumCoder::Encode(block, rank_sb);

    uint64_t new_size = Util::Floor(offset_ + len, kSmallBlockSize);
    if (new_size > bits_.size())
    {
        bits_.push_back(0);
    }
    Util::SetSlice(bits_, offset_, len, code);
    offset_ += len;

    if (num_ % kLargeBlockSize == 0)
    {
        rank_blocks_.push_back(one_num_);
        pointer_blocks_.push_back(offset_);
    }
}

void RSDic::Clear()
{
    num_ = 0;
    one_num_ = 0;
    vector<uint64_t>().swap(bits_);
    vector<rsdic_uint>().swap(pointer_blocks_);
    vector<rsdic_uint>().swap(rank_blocks_);
    vector<rsdic_uint>().swap(select_one_inds_);
    vector<rsdic_uint>().swap(select_zero_inds_);
    vector<uint8_t>().swap(rank_small_blocks_);
}

bool RSDic::GetBit(uint64_t pos) const
{
    uint64_t lblock = pos / kLargeBlockSize;
    uint64_t pointer = pointer_blocks_[lblock];
    uint64_t sblock = pos / kSmallBlockSize;
    for (uint64_t i = lblock * kSmallBlockPerLargeBlock; i < sblock; ++i)
    {
        pointer += EnumCoder::Len(rank_small_blocks_[i]);
    }
    uint64_t rank_sb = rank_small_blocks_[sblock];
    uint64_t code = Util::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    return EnumCoder::GetBit(code, rank_sb, pos % kSmallBlockSize);

}

uint64_t RSDic::Rank0(uint64_t pos) const
{
    return pos - Rank1(pos);
}

uint64_t RSDic::Rank1(uint64_t pos) const
{
    uint64_t lblock = pos / kLargeBlockSize;
    uint64_t pointer = pointer_blocks_[lblock];
    uint64_t sblock = pos / kSmallBlockSize;
    uint64_t rank = rank_blocks_[lblock];
    uint64_t rank_sb;
    for (uint64_t i = lblock * kSmallBlockPerLargeBlock; i < sblock; ++i)
    {
        rank_sb = rank_small_blocks_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_small_blocks_[sblock];
    uint64_t code = Util::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    rank += EnumCoder::Rank(code, rank_sb, pos % kSmallBlockSize);
    return rank;
}

uint64_t RSDic::Rank(uint64_t pos, bool bit) const
{
    if (bit) return Rank1(pos);
    else return pos - Rank1(pos);
}

uint64_t RSDic::Select0(uint64_t ind) const
{
    uint64_t select_ind = ind / kSelectBlockSize;
    uint64_t lblock = select_zero_inds_[select_ind];
    for (; lblock < rank_blocks_.size(); ++lblock)
    {
        if (lblock * kLargeBlockSize - rank_blocks_[lblock] > ind) break;
    }
    --lblock;

    uint64_t sblock = lblock * kSmallBlockPerLargeBlock;
    uint64_t pointer = pointer_blocks_[lblock];
    uint64_t remain = ind - lblock * kLargeBlockSize + rank_blocks_[lblock] + 1;

    uint64_t rank_sb;
    for (; sblock < rank_small_blocks_.size(); ++sblock)
    {
        rank_sb = kSmallBlockSize - rank_small_blocks_[sblock];
        if (remain <= rank_sb) break;
        remain -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_small_blocks_[sblock];
    uint64_t code = Util::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    return sblock * kSmallBlockSize + EnumCoder::Select0(code, rank_sb, remain);
}

uint64_t RSDic::Select1(uint64_t ind) const
{
    uint64_t select_ind = ind / kSelectBlockSize;
    uint64_t lblock = select_one_inds_[select_ind];
    for (; lblock < rank_blocks_.size(); ++lblock)
    {
        if (ind < rank_blocks_[lblock]) break;
    }
    --lblock;

    uint64_t sblock = lblock * kSmallBlockPerLargeBlock;
    uint64_t pointer = pointer_blocks_[lblock];
    uint64_t remain = ind - rank_blocks_[lblock] + 1;

    uint64_t rank_sb;
    for (; sblock < rank_small_blocks_.size(); ++sblock)
    {
        rank_sb = rank_small_blocks_[sblock];
        if (remain <= rank_sb) break;
        remain -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_small_blocks_[sblock];
    uint64_t code = Util::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    return sblock * kSmallBlockSize + EnumCoder::Select1(code, rank_sb, remain);
}

uint64_t RSDic::Select(uint64_t ind, bool bit) const
{
    if (bit) return Select1(ind);
    else return Select0(ind);
}

void RSDic::Save(ostream& os) const
{
    Save(os, bits_);
    Save(os, pointer_blocks_);
    Save(os, rank_blocks_);
    Save(os, select_one_inds_);
    Save(os, select_zero_inds_);
    Save(os, rank_small_blocks_);
    os.write((const char*)&num_, sizeof(num_));
    os.write((const char*)&one_num_, sizeof(one_num_));
}

void RSDic::Load(istream& is)
{
    Load(is, bits_);
    Load(is, pointer_blocks_);
    Load(is, rank_blocks_);
    Load(is, select_one_inds_);
    Load(is, select_zero_inds_);
    Load(is, rank_small_blocks_);
    is.read((char*)&num_, sizeof(num_));
    is.read((char*)&one_num_, sizeof(one_num_));
}

uint64_t RSDic::GetUsageBytes() const
{
    return sizeof(bits_[0]) * bits_.size()
        + sizeof(pointer_blocks_[0]) * pointer_blocks_.size()
        + sizeof(rank_blocks_[0]) * rank_blocks_.size()
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size()
        + sizeof(rank_small_blocks_[0]) * rank_small_blocks_.size()
        + sizeof(num_)
        + sizeof(one_num_);
}

bool RSDic::operator == (const RSDic& bv) const
{
    return bits_ == bv.bits_
        && pointer_blocks_ == bv.pointer_blocks_
        && rank_blocks_ == bv.rank_blocks_
        && select_one_inds_ == bv.select_one_inds_
        && select_zero_inds_ == bv.select_zero_inds_
        && rank_small_blocks_ == bv.rank_small_blocks_
        && num_ == bv.num_
        && one_num_ == bv.one_num_;
}

}
}

NS_IZENELIB_AM_END
