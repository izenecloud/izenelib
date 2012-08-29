/* rrr_bit_vector.h
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * RRR02 Bitsequence -
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

#ifndef _FM_INDEX_RRR_BIT_VECTOR_H
#define _FM_INDEX_RRR_BIT_VECTOR_H

#include "rrr_table_offset.hpp"


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

/** Implementation of Raman, Raman and Rao's [1] proposal for rank/select capable
 *  data structures, it achieves space nH_0, O(sample_rate) time for rank and O(log len)
 *  for select. The practial implementation is based on [2]
 *
 *  [1] R. Raman, V. Raman and S. Rao. Succinct indexable dictionaries with applications
 *     to encoding $k$-ary trees and multisets. SODA02.
 *  [2] F. Claude and G. Navarro. Practical Rank/Select over Arbitrary Sequences. SPIRE08.
 *
 *  @author Francisco Claude
 */
class RRRBitVector
{
public:
    enum
    {
        // block size can't be changed in this implementation
        // it would require more than just changing the constant
        BLOCK_SIZE = 15,
        DEFAULT_SAMPLING = 32
    };

    /** Protected constructor for loaders, you have to initialize data before
     * using an object built with this constructor.
     */
    explicit RRRBitVector(uint32_t sample_rate = DEFAULT_SAMPLING);

    /** Builds an RRR bitsequence.
     * @param bitseq bitstring
     * @param len length of the bitstring
     * @param sample_rate sampling for partial sums
     */
    RRRBitVector(const std::vector<uint32_t> &bitseq, size_t len, uint32_t sample_rate = DEFAULT_SAMPLING);

    ~RRRBitVector();

    /** Internal building function, same parameters as the base constructor. */
    void build(const std::vector<uint32_t> &bitseq, size_t len);

    bool access(const size_t i) const;

    bool access(const size_t i, size_t &r) const;

    size_t rank0(const size_t i) const;

    size_t rank1(const size_t i) const;

    size_t rank(bool b, const size_t i) const;

    size_t select0(const size_t i) const;

    size_t select1(const size_t i) const;

    size_t select(bool b, const size_t i) const;

    size_t count0() const;

    size_t count1() const;

    size_t length() const;

    size_t getSize() const;

    /** Creates a new sampling for the queries */
    void create_sampling();

    void save(std::ostream &os) const;

    void load(std::istream &is);

    /** Frees the space required by the table E, which is static and global
     *  to all instances.
     */
    static void delete_E()
    {
        delete E;
    }

private:
    /** Length of the bitstring */
    size_t length_;
    /** Number of ones in the bitstring */
    size_t ones_;
    /** Classes and offsets */
    std::vector<uint32_t> Cv_, Ov_;
    /** Length of Cv_ and Ov_ (in uints) */
    uint32_t C_len_, O_len_;
    /** Bits required per field for Cv_ and in total for Ov_ */
    uint32_t C_field_bits_, O_bits_len_;
    /** Cv_ and Ov_ samplings */
    std::vector<uint32_t> C_sampling_, O_pos_;
    /** Length of the samplings */
    uint32_t C_sampling_len_, O_pos_len_;
    /** Lenght in bits per field */
    uint32_t C_sampling_field_bits_, O_pos_field_bits_;
    /** Sample rate */
    uint32_t sample_rate_;

    static RRRTableOffset *E;
};

}
}

NS_IZENELIB_AM_END

#endif
