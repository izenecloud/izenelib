/* rrr_bit_vector.cpp
 * Copyright (Cv_) 2008, Francisco Claude, all rights reserved.
 *
 * RRRBitVector definition
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

#include <am/succinct/fm-index/rrr_bit_vector.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

RRRTableOffset *RRRBitVector::E = NULL;

RRRBitVector::RRRBitVector(uint32_t sample_rate)
    : length_(), ones_()
    , C_len_(), O_len_()
    , C_field_bits_(), O_bits_len_()
    , C_sampling_len_(), O_pos_len_()
    , C_sampling_field_bits_(), O_pos_field_bits_()
    , sample_rate_(sample_rate)
{
    if (!E) E = new RRRTableOffset(BLOCK_SIZE);
    E->use();
}

RRRBitVector::RRRBitVector(const std::vector<uint32_t> &bitseq, size_t len, uint32_t sample_rate)
    : length_(), ones_()
    , C_len_(), O_len_()
    , C_field_bits_(), O_bits_len_()
    , C_sampling_len_(), O_pos_len_()
    , C_sampling_field_bits_(), O_pos_field_bits_()
    , sample_rate_(sample_rate)
{
    if (!E) E = new RRRTableOffset(BLOCK_SIZE);
    E->use();
    build(bitseq, len);
}

RRRBitVector::~RRRBitVector()
{
    E = E->unuse();
}

void RRRBitVector::build(const std::vector<uint32_t> &bitseq, size_t len)
{
    // Table Cv_
    length_ = len;
    C_len_ = len / BLOCK_SIZE + (len % BLOCK_SIZE != 0);
    C_field_bits_ = bits((uint32_t)BLOCK_SIZE);
    Cv_.resize(uint32_len(C_len_, C_field_bits_));
    O_bits_len_ = 0;
    for (size_t i = 0; i < C_len_; ++i)
    {
        uint32_t value = popcount(get_var_field(bitseq, i * BLOCK_SIZE, std::min(len - 1, (i + 1) * BLOCK_SIZE - 1)));
        assert(value <= BLOCK_SIZE);
        set_field(Cv_, C_field_bits_, i, value);
        ones_ += value;
        O_bits_len_ += E->get_log2binomial(BLOCK_SIZE, value);
    }

    // Table Ov_
    O_len_ = uint32_len(1, O_bits_len_);
    Ov_.resize(O_len_);
    size_t O_pos = 0;
    for (size_t i = 0; i < C_len_; ++i)
    {
        uint32_t value = (uint16_t)get_var_field(bitseq, i * BLOCK_SIZE, std::min(len - 1, (i + 1) * BLOCK_SIZE - 1));
        set_var_field(Ov_, O_pos, O_pos + E->get_log2binomial(BLOCK_SIZE, popcount(value)) - 1, E->compute_offset((uint16_t)value));
        O_pos += E->get_log2binomial(BLOCK_SIZE, popcount(value));
    }

    create_sampling();
}

void RRRBitVector::create_sampling()
{
    // Sampling for Cv_
    C_sampling_len_ = C_len_ / sample_rate_ + 2;
    C_sampling_field_bits_ = bits(ones_);
    C_sampling_.clear();
    C_sampling_.resize(std::max((uint32_t)1, uint32_len(C_sampling_len_, C_sampling_field_bits_)));
    uint32_t sum = 0;
    for (uint32_t i = 0; i < C_len_; ++i)
    {
        if (i % sample_rate_ == 0)
            set_field(C_sampling_, C_sampling_field_bits_, i / sample_rate_, sum);
        sum += get_field(Cv_, C_field_bits_, i);
    }
    for (uint32_t i = (C_len_ - 1) / sample_rate_ + 1; i < C_sampling_len_; ++i)
        set_field(C_sampling_, C_sampling_field_bits_, i, sum);

    // Sampling for Ov_ (table S) (Code separated from previous construction for readability)
    O_pos_len_ = C_len_ / sample_rate_ + 1;
    O_pos_field_bits_ = bits(O_bits_len_);
    O_pos_.clear();
    O_pos_.resize(uint32_len(O_pos_len_, O_pos_field_bits_));
    uint32_t pos = 0;
    for (uint32_t i = 0; i < C_len_; ++i)
    {
        if (i % sample_rate_ == 0)
            set_field(O_pos_, O_pos_field_bits_, i / sample_rate_, pos);
        pos += E->get_log2binomial(BLOCK_SIZE, get_field(Cv_, C_field_bits_, i));
    }
}

bool RRRBitVector::access(const size_t i) const
{
    size_t nearest_sampled_value = i / BLOCK_SIZE / sample_rate_;
    size_t pos_O = get_field(O_pos_, O_pos_field_bits_, nearest_sampled_value);
    size_t pos = i / BLOCK_SIZE;
    assert(pos <= C_len_);
    for (size_t k = nearest_sampled_value * sample_rate_; k < pos; ++k)
    {
        size_t aux = get_field(Cv_, C_field_bits_, k);
        pos_O += E->get_log2binomial(BLOCK_SIZE, aux);
    }
    size_t c = get_field(Cv_, C_field_bits_, pos);
    return ((1 << (i % BLOCK_SIZE)) & E->short_bitmap(c, get_var_field(Ov_, pos_O, pos_O + E->get_log2binomial(BLOCK_SIZE, c) - 1))) != 0;
}

bool RRRBitVector::access(const size_t i, size_t &r) const
{
    if (i + 1 == 0) return 0;
    size_t nearest_sampled_value = i / BLOCK_SIZE / sample_rate_;
    size_t sum = get_field(C_sampling_, C_sampling_field_bits_, nearest_sampled_value);
    size_t pos_O = get_field(O_pos_, O_pos_field_bits_, nearest_sampled_value);
    size_t pos = i / BLOCK_SIZE;
    size_t k = nearest_sampled_value * sample_rate_;
    if (k % 2 == 1 && k < pos)
    {
        size_t aux = get_field(Cv_, C_field_bits_, k);
        sum += aux;
        pos_O += E->get_log2binomial(BLOCK_SIZE, aux);
        ++k;
    }
    unsigned char *a = (unsigned char *)&Cv_[0];
    size_t mask = 0x0F;
    a += k / 2;
    while (k < (size_t)std::max(0, (int32_t)pos - 1))
    {
        assert((*a & mask) == get_field(Cv_, C_field_bits_, k));
        assert(*a / 16 == get_field(Cv_, C_field_bits_, k + 1));
        sum += (*a & mask) + *a / 16;
        pos_O += E->get_log2binomial(BLOCK_SIZE, *a & mask) + E->get_log2binomial(BLOCK_SIZE, *a / 16);
        ++a;
        k += 2;
    }
    if (k < pos)
    {
        size_t aux = get_field(Cv_, C_field_bits_, k);
        sum += aux;
        pos_O += E->get_log2binomial(BLOCK_SIZE, aux);
        ++k;
    }
    size_t c = get_field(Cv_, C_field_bits_, pos);
    short v = E->short_bitmap(c, get_var_field(Ov_, pos_O, pos_O + E->get_log2binomial(BLOCK_SIZE, c) - 1));
    sum += popcount(((2 << (i % BLOCK_SIZE)) - 1) & v);
    r = sum;
    if (((1 << (i % BLOCK_SIZE)) & v) != 0)
    {
        return true;
    }
    else
    {
        r = i - r + 1;
        return false;
    }
}

size_t RRRBitVector::rank0(size_t i) const
{
    if (i == (size_t)-1) return 0;
    return i + 1 - rank1(i);
}

size_t RRRBitVector::rank1(size_t i) const
{
    if (i == (size_t)-1) return 0;
    size_t nearest_sampled_value = i / BLOCK_SIZE / sample_rate_;
    size_t sum = get_field(C_sampling_, C_sampling_field_bits_, nearest_sampled_value);
    size_t pos_O = get_field(O_pos_, O_pos_field_bits_, nearest_sampled_value);
    size_t pos = i / BLOCK_SIZE;
    size_t k = nearest_sampled_value * sample_rate_;
    if (k % 2 == 1 && k < pos)
    {
        uint32_t aux = get_field(Cv_, C_field_bits_, k);
        sum += aux;
        pos_O += E->get_log2binomial(BLOCK_SIZE, aux);
        ++k;
    }
    unsigned char *a = (unsigned char *)&Cv_[0];
    size_t mask = 0x0F;
    a += k / 2;
    while (k < (size_t)std::max(0, (int32_t)pos - 1))
    {
        assert((*a & mask) == get_field(Cv_, C_field_bits_, k));
        assert(*a / 16 == get_field(Cv_, C_field_bits_, k + 1));
        sum += (*a & mask) + *a / 16;
        pos_O += E->get_log2binomial(BLOCK_SIZE, *a & mask) + E->get_log2binomial(BLOCK_SIZE, *a / 16);
        ++a;
        k += 2;
    }
    if (k < pos)
    {
        size_t aux = get_field(Cv_, C_field_bits_, k);
        sum += aux;
        pos_O += E->get_log2binomial(BLOCK_SIZE, aux);
        ++k;
    }
    size_t c = get_field(Cv_, C_field_bits_, pos);
    sum += popcount(((2 << (i % BLOCK_SIZE)) - 1) & E->short_bitmap(c, get_var_field(Ov_, pos_O, pos_O + E->get_log2binomial(BLOCK_SIZE, c) - 1)));
    return sum;
}

size_t RRRBitVector::rank(bool b, size_t i) const
{
    if (b) return rank1(i);
    else return rank0(i);
}

size_t RRRBitVector::select0(size_t i) const
{
    if (i == 0 || i > length_ - ones_) return -1;

    // Search over partial sums
    size_t start = 0;
    size_t end = C_sampling_len_ - 1;
    size_t med, acc = 0, pos;
    while (start < end - 1)
    {
        med = (start + end) / 2;
        acc = med * sample_rate_ * BLOCK_SIZE - get_field(C_sampling_, C_sampling_field_bits_, med);
        if (acc < i)
        {
            if (med == start) break;
            start = med;
        }
        else
        {
            if (end == 0) break;
            end = med - 1;
        }
    }
    acc = get_field(C_sampling_, C_sampling_field_bits_, start);
    while (start < C_len_ - 1 && acc + sample_rate_ * BLOCK_SIZE == get_field(C_sampling_, C_sampling_field_bits_, start + 1))
    {
        ++start;
        acc += sample_rate_ * BLOCK_SIZE;
    }
    acc = start * sample_rate_ * BLOCK_SIZE - acc;
    pos = start * sample_rate_;
    size_t pos_O = get_field(O_pos_, O_pos_field_bits_, start);

    // Sequential search over Cv_
    size_t s = 0;
    for (; pos < C_len_; ++pos)
    {
        s = get_field(Cv_, C_field_bits_, pos);
        if (acc + BLOCK_SIZE - s >= i) break;
        pos_O += E->get_log2binomial(BLOCK_SIZE, s);
        acc += BLOCK_SIZE - s;
    }
    pos = pos * BLOCK_SIZE;

    // Search inside the block
    while (acc < i)
    {
        size_t new_posO = pos_O + E->get_log2binomial(BLOCK_SIZE, s);
        size_t block = E->short_bitmap(s, get_var_field(Ov_, pos_O, new_posO - 1));
        pos_O = new_posO;
        new_posO = 0;
        while (acc < i && new_posO < BLOCK_SIZE)
        {
            ++pos;
            ++new_posO;
            acc += (block & 1) == 0 ? 1 : 0;
            block = block / 2;
        }
    }
    --pos;
    assert(acc == i);
    assert(rank0(pos) == i);
    assert(!access(pos));
    return pos;
}

size_t RRRBitVector::select1(size_t i) const
{
    if (i == 0 || i > ones_) return -1;
    // Search over partial sums
    size_t start = 0;
    size_t end = C_sampling_len_ - 1;
    size_t med, acc = 0, pos;
    while (start < end - 1)
    {
        med = (start + end) / 2;
        acc = get_field(C_sampling_, C_sampling_field_bits_, med);
        if (acc < i)
        {
            if (med == start) break;
            start = med;
        }
        else
        {
            if (end == 0) break;
            end = med - 1;
        }
    }
    acc = get_field(C_sampling_, C_sampling_field_bits_, start);
    while (start < C_len_ - 1 && acc == get_field(C_sampling_, C_sampling_field_bits_, start + 1))
        ++start;
    pos = start * sample_rate_;
    size_t pos_O = get_field(O_pos_, O_pos_field_bits_, start);
    acc = get_field(C_sampling_, C_sampling_field_bits_, start);

    // Sequential search over Cv_
    size_t s = 0;
    for (; pos < C_len_; ++pos)
    {
        s = get_field(Cv_, C_field_bits_, pos);
        if (acc + s >= i) break;
        pos_O += E->get_log2binomial(BLOCK_SIZE, s);
        acc += s;
    }
    pos = pos * BLOCK_SIZE;

    // Search inside the block
    while (acc < i)
    {
        size_t new_posO = pos_O + E->get_log2binomial(BLOCK_SIZE, s);
        size_t block = E->short_bitmap(s, get_var_field(Ov_, pos_O, new_posO - 1));
        pos_O = new_posO;
        new_posO = 0;
        while (acc < i && new_posO < BLOCK_SIZE)
        {
            ++pos;
            ++new_posO;
            acc += (block & 1) != 0 ? 1 : 0;
            block = block / 2;
        }
    }
    --pos;
    assert(acc == i);
    assert(rank1(pos) == i);
    assert(access(pos));
    return pos;
}

size_t RRRBitVector::select(bool b, size_t i) const
{
    if (b) return select1(i);
    else return select0(i);
}

size_t RRRBitVector::count0() const
{
    return length_ - ones_;
}

size_t RRRBitVector::count1() const
{
    return ones_;
}

size_t RRRBitVector::length() const
{
    return length_;
}

size_t RRRBitVector::getSize() const
{
    // we consider E to be free (64K shared among all the RRR02 bitmaps)
    return sizeof(RRRBitVector)
        + sizeof(Cv_[0]) * Cv_.size()
        + sizeof(Ov_[0]) * Ov_.size()
        + sizeof(C_sampling_[0]) * C_sampling_.size()
        + sizeof(O_pos_[0]) * O_pos_.size();
}

void RRRBitVector::save(std::ostream &ostr) const
{
    ostr.write((const char *)&length_,                sizeof(length_));
    ostr.write((const char *)&ones_,                  sizeof(ones_));
    ostr.write((const char *)&C_len_,                 sizeof(C_len_));
    ostr.write((const char *)&O_len_,                 sizeof(O_len_));
    ostr.write((const char *)&C_field_bits_,          sizeof(C_field_bits_));
    ostr.write((const char *)&O_bits_len_,            sizeof(O_bits_len_));
    ostr.write((const char *)&C_sampling_len_,        sizeof(C_sampling_len_));
    ostr.write((const char *)&O_pos_len_,             sizeof(O_pos_len_));
    ostr.write((const char *)&C_sampling_field_bits_, sizeof(C_sampling_field_bits_));
    ostr.write((const char *)&O_pos_field_bits_,      sizeof(O_pos_field_bits_));
    ostr.write((const char *)&sample_rate_,           sizeof(sample_rate_));

    ostr.write((const char *)&Cv_[0],                 sizeof(Cv_[0]) * Cv_.size());
    ostr.write((const char *)&Ov_[0],                 sizeof(Ov_[0]) * Ov_.size());
}

void RRRBitVector::load(std::istream &istr)
{
    istr.read((char *)&length_,                sizeof(length_));
    istr.read((char *)&ones_,                  sizeof(ones_));
    istr.read((char *)&C_len_,                 sizeof(C_len_));
    istr.read((char *)&O_len_,                 sizeof(O_len_));
    istr.read((char *)&C_field_bits_,          sizeof(C_field_bits_));
    istr.read((char *)&O_bits_len_,            sizeof(O_bits_len_));
    istr.read((char *)&C_sampling_len_,        sizeof(C_sampling_len_));
    istr.read((char *)&O_pos_len_,             sizeof(O_pos_len_));
    istr.read((char *)&C_sampling_field_bits_, sizeof(C_sampling_field_bits_));
    istr.read((char *)&O_pos_field_bits_,      sizeof(O_pos_field_bits_));
    istr.read((char *)&sample_rate_,           sizeof(sample_rate_));

    Cv_.resize(uint32_len(C_len_, C_field_bits_));
    istr.read((char *)&Cv_[0],                 sizeof(Cv_[0]) * Cv_.size());
    Ov_.resize(O_len_);
    istr.read((char *)&Ov_[0],                 sizeof(Ov_[0]) * Ov_.size());

    create_sampling();
}

}
}

NS_IZENELIB_AM_END
