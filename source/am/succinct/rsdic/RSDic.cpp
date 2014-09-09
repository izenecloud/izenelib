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
#include <util/mem_utils.h>


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

void RSDic::build(const std::vector<uint64_t>& bv, size_t len)
{
    size_t size = len / kLargeBlockSize + 1;
    rank_blocks_.reset(cachealign_alloc<RankBlock>(size), cachealign_deleter());
    memset(rank_blocks_.get(), 0, size * sizeof(rank_blocks_[0]));

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
        buildBlock_(bv[i], kBlockSize, rank_blocks_[rb_index].subrank_[rb_offset], global_offset);

        if (++rb_offset == kBlockPerLargeBlock)
        {
            rb_offset = 0;
            RankBlock& rb = rank_blocks_[++rb_index];
            rb.pointer_ = global_offset;
            rb.rank_ = one_num_;
        }
    }

    if (offset)
    {
        buildBlock_(bv[index] & ((1ULL << offset) - 1), offset, rank_blocks_[rb_index].subrank_[rb_offset], global_offset);
    }

    if (bits_.capacity() > bits_.size())
    {
        std::vector<uint64_t>(bits_).swap(bits_);
    }

    if (support_select_)
    {
        select_one_inds_.push_back(num_ / kLargeBlockSize + 1);
        select_zero_inds_.push_back(num_ / kLargeBlockSize + 1);

        if (select_one_inds_.capacity() > select_one_inds_.size())
        {
            std::vector<size_t>(select_one_inds_).swap(select_one_inds_);
        }
        if (select_zero_inds_.capacity() > select_zero_inds_.size())
        {
            std::vector<size_t>(select_zero_inds_).swap(select_zero_inds_);
        }
    }
}

void RSDic::buildBlock_(uint64_t block, size_t offset, uint8_t& rank_sb, size_t& global_offset)
{
    rank_sb = SuccinctUtils::popcount(block);

    if (support_select_)
    {
        if (one_num_ % kSelectBlockSize + rank_sb >= kSelectBlockSize)
        {
            select_one_inds_.push_back(num_ / kLargeBlockSize + 1);
        }
        if ((num_ - one_num_) % kSelectBlockSize + offset - rank_sb >= kSelectBlockSize)
        {
            select_zero_inds_.push_back(num_ / kLargeBlockSize + 1);
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

void RSDic::clear()
{
    num_ = 0;
    one_num_ = 0;

    std::vector<uint64_t>().swap(bits_);
    rank_blocks_.reset();

    if (support_select_)
    {
        std::vector<size_t>().swap(select_one_inds_);
        std::vector<size_t>().swap(select_zero_inds_);
    }
}

bool RSDic::access(size_t pos) const
{
    __assert(pos < num_);

    size_t lblock = pos / kLargeBlockSize;
    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    size_t sblock = pos % kLargeBlockSize / kBlockSize;

    for (size_t i = 0; i < sblock; ++i)
    {
        pointer += EnumCoder::Len(rb.subrank_[i]);
    }
    size_t rank_sb = rb.subrank_[sblock];

    return EnumCoder::GetBit(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, pos % kBlockSize);
}

bool RSDic::access(size_t pos, size_t& rank) const
{
    __assert(pos < num_);

    size_t lblock = pos / kLargeBlockSize;
    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    size_t sblock = pos % kLargeBlockSize / kBlockSize;
    rank = rb.rank_;

    size_t rank_sb;
    for (size_t i = 0; i < sblock; ++i)
    {
        rank_sb = rb.subrank_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rb.subrank_[sblock];

    bool bit = EnumCoder::GetBit(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, pos % kBlockSize, rank_sb);
    rank = bit ? rank + rank_sb : pos - rank - rank_sb;
    return bit;
}

size_t RSDic::rank0(size_t pos) const
{
    return pos - rank1(pos);
}

size_t RSDic::rank1(size_t pos) const
{
    __assert(pos <= num_);

    size_t lblock = pos / kLargeBlockSize;
    const RankBlock& rb = rank_blocks_[lblock];
    size_t pointer = rb.pointer_;
    size_t sblock = pos % kLargeBlockSize / kBlockSize;
    size_t rank = rb.rank_;

    size_t rank_sb;
    for (size_t i = 0; i < sblock; ++i)
    {
        rank_sb = rb.subrank_[i];
        rank += rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }
    rank_sb = rb.subrank_[sblock];

    return rank + EnumCoder::Rank(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, pos % kBlockSize);
}

size_t RSDic::rank(size_t pos, bool bit) const
{
    return bit ? rank1(pos) : rank0(pos);
}

size_t RSDic::select0(size_t ind) const
{
    __assert(support_select_ && ind < num_ - one_num_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t low = select_zero_inds_[select_ind];
    size_t high = select_zero_inds_[select_ind + 1];

    while (low + kSelectLinearFallback < high)
    {
        size_t mid = (low + high) / 2;
        asm("cmpq %3, %2\n\tcmovge %4, %0\n\tcmovl %5, %1"
                : "+r"(low), "+r"(high)
                : "r"(ind), "g"(mid * kLargeBlockSize - rank_blocks_[mid].rank_), "g"(mid + 1), "g"(mid));
    }

    for (; low < high && low * kLargeBlockSize - rank_blocks_[low].rank_ <= ind; ++low);

    __assert(low > 0);
    const RankBlock& rb = rank_blocks_[--low];
    size_t pointer = rb.pointer_;
    ind -= low * kLargeBlockSize - rb.rank_;

    size_t rank_sb;
    size_t sblock = 0;
    for (;; ++sblock)
    {
        rank_sb = kBlockSize - rb.subrank_[sblock];
        if (ind < rank_sb) break;
        ind -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }

    return low * kLargeBlockSize + sblock * kBlockSize + EnumCoder::Select(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, ind, false);
}

size_t RSDic::select1(size_t ind) const
{
    __assert(support_select_ && ind < one_num_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t low = select_one_inds_[select_ind];
    size_t high = select_one_inds_[select_ind + 1];

    while (low + kSelectLinearFallback < high)
    {
        size_t mid = (low + high) / 2;
        asm("cmpq %3, %2\n\tcmovge %4, %0\n\tcmovl %5, %1"
                : "+r"(low), "+r"(high)
                : "r"(ind), "g"(rank_blocks_[mid].rank_), "g"(mid + 1), "g"(mid));
    }

    for (; low < high && rank_blocks_[low].rank_ <= ind; ++low);

    __assert(low > 0);
    const RankBlock& rb = rank_blocks_[--low];
    size_t pointer = rb.pointer_;
    ind -= rb.rank_;

    size_t rank_sb;
    size_t sblock = 0;
    for (;; ++sblock)
    {
        rank_sb = rb.subrank_[sblock];
        if (ind < rank_sb) break;
        ind -= rank_sb;
        pointer += EnumCoder::Len(rank_sb);
    }

    return low * kLargeBlockSize + sblock * kBlockSize + EnumCoder::Select(SuccinctUtils::GetSlice(bits_, pointer, EnumCoder::Len(rank_sb)), rank_sb, ind, true);
}

size_t RSDic::select(size_t ind, bool bit) const
{
    return bit ? select1(ind) : select0(ind);
}

void RSDic::save(std::ostream& os) const
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

void RSDic::load(std::istream& is)
{
    is.read((char*)&support_select_, sizeof(support_select_));
    is.read((char*)&num_, sizeof(num_));
    is.read((char*)&one_num_, sizeof(one_num_));

    SuccinctUtils::loadVec(is, bits_);

    size_t size = num_ / kLargeBlockSize + 1;
    rank_blocks_.reset(cachealign_alloc<RankBlock>(size), cachealign_deleter());
    memset(rank_blocks_.get(), 0, size * sizeof(rank_blocks_[0]));
    is.read((char *)&rank_blocks_[0], size * sizeof(rank_blocks_[0]));

    if (support_select_)
    {
        SuccinctUtils::loadVec(is, select_one_inds_);
        SuccinctUtils::loadVec(is, select_zero_inds_);
    }
}

size_t RSDic::allocSize() const
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
