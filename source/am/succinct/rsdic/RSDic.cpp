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

#include <am/succinct/rsdic/EnumCoder.hpp>
#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/utils.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace rsdic
{

RSDic::RSDic(bool support_select)
    : support_select_(support_select)
    , num_(), one_num_()
{
}

RSDic::~RSDic()
{
}

void RSDic::Build(const std::vector<uint64_t>& bv, size_t len)
{
    pointer_blocks_.reserve(len / kLargeBlockSize + 1);
    rank_large_blocks_.reserve(len / kLargeBlockSize + 1);
    rank_blocks_.reserve(len / kBlockSize + 1);

    pointer_blocks_.push_back(0);
    rank_large_blocks_.push_back(0);
    if (support_select_)
    {
        select_one_inds_.push_back(0);
        select_zero_inds_.push_back(0);
    }

    size_t global_offset = 0;
    size_t index = len / kBlockSize;
    size_t offset = len % kBlockSize;

    for (size_t i = 0; i < index; ++i)
    {
        BuildBlock_(bv[i], kBlockSize, global_offset);

        if ((i + 1) % kBlockPerLargeBlock == 0)
        {
            pointer_blocks_.push_back(global_offset);
            rank_large_blocks_.push_back(one_num_);
        }
    }
    BuildBlock_(offset ? bv[index] & ((1LLU << offset) - 1) : 0, offset, global_offset);

    if (bits_.capacity() > bits_.size())
    {
        std::vector<uint64_t>(bits_).swap(bits_);
    }
    if (select_one_inds_.capacity() > select_one_inds_.size())
    {
        std::vector<rsdic_uint>(select_one_inds_).swap(select_one_inds_);
    }
    if (select_zero_inds_.capacity() > select_zero_inds_.size())
    {
        std::vector<rsdic_uint>(select_zero_inds_).swap(select_zero_inds_);
    }
}

void RSDic::BuildBlock_(uint64_t block, size_t offset, size_t& global_offset)
{
    size_t rank_sb = SuccinctUtils::popcount(block);

    if (support_select_)
    {
        if (one_num_ % kSelectBlockSize + rank_sb >= kSelectBlockSize)
        {
            select_one_inds_.push_back(num_ / kLargeBlockSize);
        }
        if ((num_ - one_num_) % kSelectBlockSize + offset - rank_sb >= kSelectBlockSize)
        {
            select_zero_inds_.push_back(num_ / kLargeBlockSize);
        }
    }

    num_ += offset;
    one_num_ += rank_sb;

    rank_blocks_.push_back(rank_sb);

    size_t len = EnumCoder::Len(rank_sb);
    uint64_t code;
    if (len == kBlockSize)
    {
        code = block; // use raw
    }
    else
    {
        code = EnumCoder::Encode(block, rank_sb);
    }

    size_t new_size = SuccinctUtils::Ceiling(global_offset + len, kBlockSize);
    if (new_size > bits_.size())
    {
        bits_.push_back(0);
    }
    SuccinctUtils::SetSlice(bits_, global_offset, len, code);
    global_offset += len;
}

void RSDic::Clear()
{
    num_ = 0;
    one_num_ = 0;
    std::vector<uint64_t>().swap(bits_);
    std::vector<rsdic_uint>().swap(pointer_blocks_);
    std::vector<rsdic_uint>().swap(rank_large_blocks_);
    std::vector<rsdic_uint>().swap(select_one_inds_);
    std::vector<rsdic_uint>().swap(select_zero_inds_);
    std::vector<uint8_t>().swap(rank_blocks_);
}

bool RSDic::GetBit(size_t pos) const
{
    __assert(pos < num_);

    size_t lblock = pos / kLargeBlockSize;
    size_t pointer = pointer_blocks_[lblock];
    size_t sblock = pos / kBlockSize;
    for (size_t i = lblock * kBlockPerLargeBlock; i < sblock; ++i)
    {
        pointer += EnumCoder::Len(rank_blocks_[i]);
    }
    size_t rank_sb = rank_blocks_[sblock];
    uint64_t code = SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    return EnumCoder::GetBit(code, rank_sb, pos % kBlockSize);

}

bool RSDic::GetBit(size_t pos, size_t& rank) const
{
    __assert(pos < num_);

    size_t lblock = pos / kLargeBlockSize;
    size_t pointer = pointer_blocks_[lblock];
    size_t sblock = pos / kBlockSize;
    rank = rank_large_blocks_[lblock];

    size_t rank_sb;
    for (size_t i = lblock * kBlockPerLargeBlock; i < sblock; ++i)
    {
        rank_sb = rank_blocks_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_blocks_[sblock];
    uint64_t code = SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    if (EnumCoder::GetBit(code, rank_sb, pos % kBlockSize, rank_sb))
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

size_t RSDic::Rank0(size_t pos) const
{
    return pos - Rank1(pos);
}

size_t RSDic::Rank1(size_t pos) const
{
    __assert(pos <= num_);

    size_t lblock = pos / kLargeBlockSize;
    size_t pointer = pointer_blocks_[lblock];
    size_t sblock = pos / kBlockSize;
    size_t rank = rank_large_blocks_[lblock];

    size_t rank_sb;
    for (size_t i = lblock * kBlockPerLargeBlock; i < sblock; ++i)
    {
        rank_sb = rank_blocks_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_blocks_[sblock];
    uint64_t code = SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    rank += EnumCoder::Rank(code, rank_sb, pos % kBlockSize);
    return rank;
}

size_t RSDic::Rank(size_t pos, bool bit) const
{
    if (bit) return Rank1(pos);
    else return pos - Rank1(pos);
}

size_t RSDic::Select0(size_t ind) const
{
    __assert(support_select_ && ind < num_ - one_num_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t lblock = select_zero_inds_[select_ind];
    for (; lblock < rank_large_blocks_.size(); ++lblock)
    {
        if (lblock * kLargeBlockSize - rank_large_blocks_[lblock] > ind) break;
    }
    --lblock;

    size_t sblock = lblock * kBlockPerLargeBlock;
    size_t pointer = pointer_blocks_[lblock];
    size_t remain = ind - lblock * kLargeBlockSize + rank_large_blocks_[lblock];

    size_t rank_sb;
    for (; sblock < rank_blocks_.size(); ++sblock)
    {
        rank_sb = kBlockSize - rank_blocks_[sblock];
        if (remain < rank_sb) break;
        remain -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_blocks_[sblock];
    uint64_t code = SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    return sblock * kBlockSize + EnumCoder::Select0(code, rank_sb, remain);
}

size_t RSDic::Select1(size_t ind) const
{
    __assert(support_select_ && ind < one_num_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t lblock = select_one_inds_[select_ind];
    for (; lblock < rank_large_blocks_.size(); ++lblock)
    {
        if (ind < rank_large_blocks_[lblock]) break;
    }
    --lblock;

    size_t sblock = lblock * kBlockPerLargeBlock;
    size_t pointer = pointer_blocks_[lblock];
    size_t remain = ind - rank_large_blocks_[lblock];

    size_t rank_sb;
    for (; sblock < rank_blocks_.size(); ++sblock)
    {
        rank_sb = rank_blocks_[sblock];
        if (remain < rank_sb) break;
        remain -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rank_blocks_[sblock];
    uint64_t code = SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb));
    return sblock * kBlockSize + EnumCoder::Select1(code, rank_sb, remain);
}

size_t RSDic::Select(size_t ind, bool bit) const
{
    if (bit) return Select1(ind);
    else return Select0(ind);
}

void RSDic::Save(std::ostream& os) const
{
    os.write((const char*)&support_select_, sizeof(support_select_));
    os.write((const char*)&num_, sizeof(num_));
    os.write((const char*)&one_num_, sizeof(one_num_));
    Save(os, bits_);
    Save(os, pointer_blocks_);
    Save(os, rank_large_blocks_);
    if (support_select_)
    {
        Save(os, select_one_inds_);
        Save(os, select_zero_inds_);
    }
    Save(os, rank_blocks_);
}

void RSDic::Load(std::istream& is)
{
    is.read((char*)&support_select_, sizeof(support_select_));
    is.read((char*)&num_, sizeof(num_));
    is.read((char*)&one_num_, sizeof(one_num_));
    Load(is, bits_);
    Load(is, pointer_blocks_);
    Load(is, rank_large_blocks_);
    if (support_select_)
    {
        Load(is, select_one_inds_);
        Load(is, select_zero_inds_);
    }
    Load(is, rank_blocks_);
}

size_t RSDic::GetUsageBytes() const
{
    return sizeof(RSDic)
        + sizeof(bits_[0]) * bits_.size()
        + sizeof(pointer_blocks_[0]) * pointer_blocks_.size()
        + sizeof(rank_large_blocks_[0]) * rank_large_blocks_.size()
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size()
        + sizeof(rank_blocks_[0]) * rank_blocks_.size();
}

bool RSDic::operator == (const RSDic& bv) const
{
    return support_select_ == bv.support_select_
        && num_ == bv.num_
        && one_num_ == bv.one_num_
        && bits_ == bv.bits_
        && pointer_blocks_ == bv.pointer_blocks_
        && rank_large_blocks_ == bv.rank_large_blocks_
        && select_one_inds_ == bv.select_one_inds_
        && select_zero_inds_ == bv.select_zero_inds_
        && rank_blocks_ == bv.rank_blocks_;
}

}
}

NS_IZENELIB_AM_END
