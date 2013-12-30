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
    size_t size = len / kLargeBlockSize + 1;
    rank_blocks_.reset(SuccinctUtils::cachealign_alloc<RankBlock>(CACHELINE_SIZE, size));

    if (support_select_)
    {
        select_one_inds_.push_back(0);
        select_zero_inds_.push_back(0);
    }

    size_t global_offset = 0;
    size_t index = len / kBlockSize;
    size_t offset = len % kBlockSize;
    size_t rb_index = 0;
    size_t rb_offset = 0;

    for (size_t i = 0; i < index; ++i)
    {
        BuildBlock_(bv[i], kBlockSize, rank_blocks_[rb_index].small_blocks_[rb_offset], global_offset);

        if (++rb_offset == kBlockPerLargeBlock)
        {
            rb_offset = 0;
            RankBlock& rb = rank_blocks_[++rb_index];
            rb.pointer_ = global_offset;
            rb.large_block_ = one_num_;
        }
    }

    if (offset)
    {
        BuildBlock_(bv[index] & ((1LLU << offset) - 1), offset, rank_blocks_[rb_index].small_blocks_[rb_offset], global_offset);
    }

    if (bits_.capacity() > bits_.size())
    {
        std::vector<uint64_t>(bits_).swap(bits_);
    }

    if (support_select_)
    {
        if (select_one_inds_.capacity() > select_one_inds_.size())
        {
            std::vector<rsdic_uint>(select_one_inds_).swap(select_one_inds_);
        }
        if (select_zero_inds_.capacity() > select_zero_inds_.size())
        {
            std::vector<rsdic_uint>(select_zero_inds_).swap(select_zero_inds_);
        }
    }
}

void RSDic::BuildBlock_(uint64_t block, size_t offset, uint8_t& rank_sb, size_t& global_offset)
{
    rank_sb = SuccinctUtils::popcount(block);

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

    size_t len = EnumCoder::Len(rank_sb);
    uint64_t code = (len == kBlockSize) ? block : EnumCoder::Encode(block, rank_sb);

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
    rank_blocks_.reset();

    if (support_select_)
    {
        std::vector<rsdic_uint>().swap(select_one_inds_);
        std::vector<rsdic_uint>().swap(select_zero_inds_);
    }
}

bool RSDic::GetBit(size_t pos) const
{
    __assert(pos < num_);

    size_t lblock = pos / kLargeBlockSize;
    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    size_t sblock = pos % kLargeBlockSize / kBlockSize;

    for (size_t i = 0; i < sblock; ++i)
    {
        pointer += EnumCoder::Len(rb.small_blocks_[i]);
    }
    size_t rank_sb = rb.small_blocks_[sblock];

    return EnumCoder::GetBit(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, pos % kBlockSize);
}

bool RSDic::GetBit(size_t pos, size_t& rank) const
{
    __assert(pos < num_);

    size_t lblock = pos / kLargeBlockSize;
    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    size_t sblock = pos % kLargeBlockSize / kBlockSize;
    rank = rb.large_block_;

    size_t rank_sb;
    for (size_t i = 0; i < sblock; ++i)
    {
        rank_sb = rb.small_blocks_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rb.small_blocks_[sblock];

    if (EnumCoder::GetBit(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, pos % kBlockSize, rank_sb))
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
    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    size_t sblock = pos % kLargeBlockSize / kBlockSize;
    size_t rank = rb.large_block_;

    size_t rank_sb;
    for (size_t i = 0; i < sblock; ++i)
    {
        rank_sb = rb.small_blocks_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rb.small_blocks_[sblock];

    return rank + EnumCoder::Rank(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, pos % kBlockSize);
}

size_t RSDic::Rank(size_t pos, bool bit) const
{
    return bit ? Rank1(pos) : Rank0(pos);
}

size_t RSDic::Select0(size_t ind) const
{
    __assert(support_select_ && ind < num_ - one_num_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t lblock = select_zero_inds_[select_ind];
    size_t size = num_ / kLargeBlockSize + 1;
    for (; lblock < size; ++lblock)
    {
        if (lblock * kLargeBlockSize - rank_blocks_[lblock].large_block_ > ind) break;
    }
    --lblock;

    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    ind -= lblock * kLargeBlockSize - rb.large_block_;

    size_t rank_sb;
    size_t sblock = 0;
    for (; sblock < kBlockPerLargeBlock; ++sblock)
    {
        rank_sb = kBlockSize - rb.small_blocks_[sblock];
        if (ind < rank_sb) break;
        ind -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }

    return lblock * kLargeBlockSize + sblock * kBlockSize + EnumCoder::Select(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, ind, false);
}

size_t RSDic::Select1(size_t ind) const
{
    __assert(support_select_ && ind < one_num_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t lblock = select_one_inds_[select_ind];
    size_t size = num_ / kLargeBlockSize + 1;
    for (; lblock < size; ++lblock)
    {
        if (rank_blocks_[lblock].large_block_ > ind) break;
    }
    --lblock;

    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    ind -= rb.large_block_;

    size_t rank_sb;
    size_t sblock = 0;
    for (; sblock < kBlockPerLargeBlock; ++sblock)
    {
        rank_sb = rb.small_blocks_[sblock];
        if (ind < rank_sb) break;
        ind -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }

    return lblock * kLargeBlockSize + sblock * kBlockSize + EnumCoder::Select(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, ind, true);
}

size_t RSDic::Select(size_t ind, bool bit) const
{
    return bit ? Select1(ind) : Select0(ind);
}

void RSDic::Save(std::ostream& os) const
{
    os.write((const char*)&support_select_, sizeof(support_select_));
    os.write((const char*)&num_, sizeof(num_));
    os.write((const char*)&one_num_, sizeof(one_num_));

    SuccinctUtils::saveVec(os, bits_);

    size_t size = num_ / kLargeBlockSize + 1;
    os.write((const char *)&rank_blocks_[0], size * sizeof(rank_blocks_[0]));

    if (support_select_)
    {
        SuccinctUtils::saveVec(os, select_one_inds_);
        SuccinctUtils::saveVec(os, select_zero_inds_);
    }
}

void RSDic::Load(std::istream& is)
{
    is.read((char*)&support_select_, sizeof(support_select_));
    is.read((char*)&num_, sizeof(num_));
    is.read((char*)&one_num_, sizeof(one_num_));

    SuccinctUtils::loadVec(is, bits_);

    size_t size = num_ / kLargeBlockSize + 1;
    rank_blocks_.reset(SuccinctUtils::cachealign_alloc<RankBlock>(CACHELINE_SIZE, size));
    is.read((char *)&rank_blocks_[0], size * sizeof(rank_blocks_[0]));

    if (support_select_)
    {
        SuccinctUtils::loadVec(is, select_one_inds_);
        SuccinctUtils::loadVec(is, select_zero_inds_);
    }
}

size_t RSDic::GetUsageBytes() const
{
    return sizeof(RSDic)
        + sizeof(bits_[0]) * bits_.size()
        + sizeof(rank_blocks_[0]) * (num_ / kLargeBlockSize + 1)
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size();
}

}
}

NS_IZENELIB_AM_END
