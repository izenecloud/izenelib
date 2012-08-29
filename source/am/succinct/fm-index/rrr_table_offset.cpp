/* rrr_table_offset.cpp
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * Table for offsets.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <am/succinct/fm-index/rrr_table_offset.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

uint32_t __indiceFunc;
uint32_t __indAccumulated;

uint32_t generateClass(std::vector<uint16_t> &bitmap, std::vector<uint16_t> &rev, uint32_t block_size, uint32_t class_offset, uint32_t inclass_offset, uint32_t pos_ini, uint32_t generated)
{
    uint32_t ret = 0;
    if (class_offset == inclass_offset)
    {
        bitmap[__indiceFunc] = generated;
        rev[generated] = __indiceFunc - __indAccumulated;
        ++__indiceFunc;
        return 1;
    }
    if (class_offset < inclass_offset)
        return 0;
    for (uint32_t i = pos_ini; i < block_size; ++i)
    {
        uint32_t tmp = generated | 1 << i;
        ret += generateClass(bitmap, rev, block_size, class_offset, inclass_offset + 1, i + 1, tmp);
    }
    return ret;
}

RRRTableOffset::RRRTableOffset(uint32_t block_size)
    : users_count_(0)
    , block_size_(block_size)
    , binomial_(block_size_ + 1)
    , log2binomial_(block_size_ + 1)
    , rev_offset_(1 << block_size_)
    , offset_class_(block_size_ + 2)
    , short_bitmaps_(1 << block_size_)
{
    for (uint32_t i = 0; i < block_size_ + 1; ++i)
    {
        binomial_[i].resize(block_size_ + 1);
        log2binomial_[i].resize(block_size_ + 1);

        binomial_[i][0] = 1;
        binomial_[i][i] = 1;
        for (uint32_t j = 1; j < i; ++j)
        {
            binomial_[i][j] = binomial_[i - 1][j - 1] + binomial_[i - 1][j];
            log2binomial_[i][j] = bits((uint32_t)binomial_[i][j] - 1);
        }
    }
    fill_tables_();
}

RRRTableOffset::~RRRTableOffset()
{
}

size_t RRRTableOffset::size()
{
    size_t ret = sizeof(RRRTableOffset);
    ret += (sizeof(binomial_[0]) + sizeof(log2binomial_[0])) * (block_size_ + 1);
    ret += (sizeof(uint32_t) + sizeof(uint16_t)) * ((block_size_ + 1) * (block_size_ + 1));
    ret += sizeof(uint16_t) * (1 << block_size_);
    ret += sizeof(uint16_t) * (block_size_ + 1);
    ret += sizeof(uint16_t) * ((1 << block_size_) + 1);
    ret += sizeof(uint16_t) * (block_size_ + 1);
    return ret;
}

void RRRTableOffset::fill_tables_()
{
    __indAccumulated = 0;
    __indiceFunc = 0;
    offset_class_[0] = 0;

    for (uint32_t i = 0; i <= block_size_; ++i)
    {
        __indAccumulated += generateClass(short_bitmaps_, rev_offset_, block_size_, i, 0, 0, 0);
        offset_class_[i + 1] = __indiceFunc;
    }
}

}
}

NS_IZENELIB_AM_END
