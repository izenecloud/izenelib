/*
 *  Copyright (c) 2011 Daisuke Okanohara
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

#ifndef IZENELIB_AM_SUCCINCT_DAG_VECTOR_BIT_VECTOR_HPP_
#define IZENELIB_AM_SUCCINCT_DAG_VECTOR_BIT_VECTOR_HPP_

#include <types.h>
#include <vector>

NS_IZENELIB_AM_BEGIN

namespace succinct{

class bit_vector
{
public:
    bit_vector() : size_(0)
    {
    }

    ~bit_vector()
    {
    }

    void clear()
    {
        std::vector<uint64_t>().swap(bits_);
        size_ = 0;
    }

    void push_back(uint64_t val, uint64_t len)
    {
        if (len == 0) return;
        bits_.resize((size_ + len - 1) / BLOCK_SIZE + 1);
        uint64_t block_ind = size_ / BLOCK_SIZE;
        uint64_t block_offset = size_ % BLOCK_SIZE;
        bits_[block_ind] |= val << block_offset;
        if (block_offset + len > BLOCK_SIZE)
        {
            bits_[block_ind+1] |= val >> (BLOCK_SIZE - block_offset);
        }
        size_ += len;
    }

    uint64_t get_bit(uint64_t pos) const
    {
        return (bits_[pos/BLOCK_SIZE] >> (pos % BLOCK_SIZE)) & 0x1LLU;
    }

    uint64_t get_bits(uint64_t pos, uint64_t len) const
    {
        if (len == 0) return 0;
        uint64_t block_ind = pos / BLOCK_SIZE;
        uint64_t block_offset = pos % BLOCK_SIZE;
        uint64_t mask = (len < BLOCK_SIZE) ?
                        ((1LLU << len) - 1) : 0xFFFFFFFFFFFFFFFFLLU;
        if (block_offset + len <= BLOCK_SIZE)
        {
            return (bits_[block_ind] >> block_offset) & mask;
        }
        else
        {
            return ((bits_[block_ind] >> block_offset) +
                    (bits_[block_ind+1] << (BLOCK_SIZE - block_offset))) & mask;
        }
    }

    uint64_t size() const
    {
        return size_;
    }

    void swap(bit_vector& bv)
    {
        bits_.swap(bv.bits_);
        std::swap(size_, bv.size_);
    }

    uint64_t get_alloc_byte_num() const
    {
        return bits_.size() * sizeof(uint64_t) + sizeof(uint64_t);
    }

private:
    static const uint64_t BLOCK_SIZE = 64;
    std::vector<uint64_t> bits_;
    uint64_t size_;
};

}
NS_IZENELIB_AM_END

#endif // IZENELIB_AM_SUCCINCT_DAG_VECTOR_BIT_VECTOR_HPP_