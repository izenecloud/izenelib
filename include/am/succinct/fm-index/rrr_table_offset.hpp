/* rrr_table_offset.h
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * Table for offsets definition.
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

#ifndef _FM_INDEX_RRR_TABLE_OFFSET_HPP
#define _FM_INDEX_RRR_TABLE_OFFSET_HPP

#include "utils.hpp"

#include <iostream>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

/** Universal table required for RRRBitVector, Raman, Raman and Rao's [1]
 *  proposal for rank/select capable data structures, it achieves space nH_0,
 *  O(sample_rate) time for rank and O(log len) for select. The practial implementation
 *  is based on [2]
 *
 *  [1] R. Raman, V. Raman and S. Rao. Succinct indexable dictionaries with applications
 *     to encoding $k$-ary trees and multisets. SODA02.
 *  [2] F. Claude and G. Navarro. Practical Rank/Select over Arbitrary Sequences. SPIRE08.
 *
 *  @author Francisco Claude
 */
class RRRTableOffset
{

public:
    /** builds a universal table, designed for block_size<=15 */
    explicit RRRTableOffset(uint32_t block_size);
    ~RRRTableOffset();

    /** Increments the counter of users for the table */
    inline void use()
    {
        ++users_count_;
    }

    /** Tells the object that the user is not going to need the table anymore. */
    inline RRRTableOffset * unuse()
    {
        --users_count_;
        if (!users_count_)
        {
            delete this;
            return NULL;
        }
        return this;
    }

    /** Computes binomial(n,k) for n,k<=block_size */
    inline uint16_t get_binomial(uint32_t n, uint32_t k)
    {
        return binomial_[n][k];
    }

    /** Computes ceil(log2(binomial(n,k))) for n,k<=block_size */
    inline uint8_t get_log2binomial(uint32_t n, uint32_t k)
    {
        return log2binomial_[n][k];
    }

    /** Returns the bitmap represented by the given class and inclass offsets */
    inline uint16_t short_bitmap(uint32_t class_offset, uint32_t inclass_offset)
    {
        if (class_offset == 0) return 0;
        if (class_offset == block_size_) return uint16_t((1LLU << block_size_) - 1);
        return short_bitmaps_[offset_class_[class_offset] + inclass_offset];
    }

    /** Returns block_size */
    inline uint32_t get_u()
    {
        return block_size_;
    }

    /** Computes the offset of the first block_size bits of a given bitstring */
    inline uint16_t compute_offset(uint16_t v)
    {
        return rev_offset_[v];
    }

    /** Returns the size of the bitmap in bytes */
    size_t size();

private:
    int32_t users_count_;
    uint32_t block_size_;
    std::vector<std::vector<uint16_t> > binomial_;
    std::vector<std::vector<uint8_t> > log2binomial_;
    std::vector<uint16_t> rev_offset_;
    std::vector<uint16_t> offset_class_;
    std::vector<uint16_t> short_bitmaps_;

    void fill_tables_();
};

}
}

NS_IZENELIB_AM_END

#endif
