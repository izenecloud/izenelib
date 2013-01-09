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
    pointer_blocks_.reserve(len / kLargeBlockSize + 1);
    rank_blocks_.reserve(len / kLargeBlockSize + 1);
    rank_small_blocks_.reserve(len / kSmallBlockSize + 1);

    pointer_blocks_.push_back(0);
    rank_blocks_.push_back(0);
    select_one_inds_.push_back(0);
    select_zero_inds_.push_back(0);

    uint64_t global_offset = 0;
    uint64_t index = len / kSmallBlockSize;
    uint64_t offset = len % kSmallBlockSize;

    for (uint64_t i = 0; i < index; ++i)
    {
        BuildBlock_(bv[i], kSmallBlockSize, global_offset);

        if ((i + 1) % kSmallBlockPerLargeBlock == 0)
        {
            pointer_blocks_.push_back(global_offset);
            rank_blocks_.push_back(one_num_);
        }
    }
    BuildBlock_(offset ? bv[index] & ((1LLU << offset) - 1) : 0, offset, global_offset);

    if (bits_.capacity() > bits_.size())
    {
        vector<uint64_t>(bits_).swap(bits_);
    }
    if (select_one_inds_.capacity() > select_one_inds_.size())
    {
        vector<rsdic_uint>(select_one_inds_).swap(select_one_inds_);
    }
    if (select_zero_inds_.capacity() > select_zero_inds_.size())
    {
        vector<rsdic_uint>(select_zero_inds_).swap(select_zero_inds_);
    }
}

void RSDic::BuildBlock_(uint64_t block, uint64_t offset, uint64_t& global_offset)
{
    uint64_t rank_sb = Util::PopCount(block);

    if (rank_sb > 0 && one_num_ % kSelectBlockSize + rank_sb >= kSelectBlockSize)
    {
        size_t remain = EnumCoder::SelectRaw(block, kSelectBlockSize - one_num_ % kSelectBlockSize);
        select_one_inds_.push_back((num_ + remain) / kLargeBlockSize);
    }
    if (offset - rank_sb > 0 && (num_ - one_num_) % kSelectBlockSize + offset - rank_sb >= kSelectBlockSize)
    {
        size_t remain = EnumCoder::SelectRaw(~block, kSelectBlockSize - (num_ - one_num_) % kSelectBlockSize);
        select_zero_inds_.push_back((num_ + remain) / kLargeBlockSize);
    }
    num_ += offset;
    one_num_ += rank_sb;

    rank_small_blocks_.push_back(rank_sb);

    uint64_t len = EnumCoder::Len(rank_sb);
    uint64_t code;
    if (len == kSmallBlockSize)
    {
        code = block; // use raw
    }
    else
    {
        code = EnumCoder::Encode(block, rank_sb);
    }

    uint64_t new_size = Util::Ceiling(global_offset + len, kSmallBlockSize);
    if (new_size > bits_.size())
    {
        bits_.push_back(0);
    }
    Util::SetSlice(bits_, global_offset, len, code);
    global_offset += len;
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

bool RSDic::GetBit(uint64_t pos, uint64_t& rank) const
{
    uint64_t lblock = pos / kLargeBlockSize;
    uint64_t pointer = pointer_blocks_[lblock];
    uint64_t sblock = pos / kSmallBlockSize;
    rank = rank_blocks_[lblock];

    uint64_t rank_sb;
    for (uint64_t i = lblock * kSmallBlockPerLargeBlock; i < sblock; ++i)
    {
        rank_sb = rank_small_blocks_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_small_blocks_[sblock];
    uint64_t code = Util::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    if (EnumCoder::GetBit(code, rank_sb, pos % kSmallBlockSize, rank_sb))
    {
        rank += rank_sb;
        return true;
    }
    else
    {
        rank = pos - rank - rank_sb;
        return false;
    }

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
    if (ind >= num_ - one_num_) return -1;

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
    if (ind >= one_num_) return -1;

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
    os.write((const char*)&num_, sizeof(num_));
    os.write((const char*)&one_num_, sizeof(one_num_));
    Save(os, bits_);
    Save(os, pointer_blocks_);
    Save(os, rank_blocks_);
    Save(os, select_one_inds_);
    Save(os, select_zero_inds_);
    Save(os, rank_small_blocks_);
}

void RSDic::Load(istream& is)
{
    is.read((char*)&num_, sizeof(num_));
    is.read((char*)&one_num_, sizeof(one_num_));
    Load(is, bits_);
    Load(is, pointer_blocks_);
    Load(is, rank_blocks_);
    Load(is, select_one_inds_);
    Load(is, select_zero_inds_);
    Load(is, rank_small_blocks_);
}

uint64_t RSDic::GetUsageBytes() const
{
    return sizeof(RSDic)
        + sizeof(bits_[0]) * bits_.size()
        + sizeof(pointer_blocks_[0]) * pointer_blocks_.size()
        + sizeof(rank_blocks_[0]) * rank_blocks_.size()
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size()
        + sizeof(rank_small_blocks_[0]) * rank_small_blocks_.size();
}

bool RSDic::operator == (const RSDic& bv) const
{
    return num_ == bv.num_
        && one_num_ == bv.one_num_
        && bits_ == bv.bits_
        && pointer_blocks_ == bv.pointer_blocks_
        && rank_blocks_ == bv.rank_blocks_
        && select_one_inds_ == bv.select_one_inds_
        && select_zero_inds_ == bv.select_zero_inds_
        && rank_small_blocks_ == bv.rank_small_blocks_;
}

}
}

NS_IZENELIB_AM_END
