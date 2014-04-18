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

#ifndef RSDIC_RSDIC_HPP_
#define RSDIC_RSDIC_HPP_

#include <am/succinct/constants.hpp>

#include <boost/shared_array.hpp>
#include <vector>
#include <iostream>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace rsdic
{

class RSDic
{
public:
    RSDic(bool support_select);
    ~RSDic();

    void Build(const std::vector<uint64_t>& bv, size_t len);
    void Clear();

    bool GetBit(size_t pos) const;
    bool GetBit(size_t pos, size_t& rank) const;

    size_t Rank0(size_t pos) const;
    size_t Rank1(size_t pos) const;
    size_t Rank(size_t pos, bool bit) const;

    size_t Select0(size_t ind) const;
    size_t Select1(size_t ind) const;
    size_t Select(size_t ind, bool bit) const;

    void Save(std::ostream& os) const;
    void Load(std::istream& is);
    size_t GetUsageBytes() const;

    bool support_select() const
    {
        return support_select_;
    }

    size_t num() const
    {
        return num_;
    }

    size_t one_num() const
    {
        return one_num_;
    }

    size_t zero_num() const
    {
        return num_ - one_num_;
    }

private:
    struct RankBlock
    {
        uint64_t pointer_;
        uint64_t rank_;
        uint8_t subrank_[kBlockPerLargeBlock];

        RankBlock() : pointer_(), rank_(), subrank_() {}
    };

    void BuildBlock_(uint64_t block, size_t offset, uint8_t& rank_sb, size_t& global_offset);

    bool support_select_;
    size_t num_;
    size_t one_num_;

    std::vector<uint64_t> bits_;
    boost::shared_array<RankBlock> rank_blocks_;

    std::vector<size_t> select_one_inds_;
    std::vector<size_t> select_zero_inds_;
};

}
}

NS_IZENELIB_AM_END

#endif // RSDIC_BITVEC_HPP_
