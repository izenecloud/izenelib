/*
 *  wavelet-matrix.cpp
 *  Copyright (c) 2012 Hiroshi Manabe
 *
 *  based on wat-array.cpp
 *  Copyright (c) 2010 Daisuke Okanohara
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

#include <queue>
#include <algorithm>
#include <am/succinct/wat_array/wavelet_matrix.hpp>

using namespace std;

namespace wavelet_matrix
{

WaveletMatrix::WaveletMatrix() : alphabet_num_(0), alphabet_bit_num_(0), length_(0)
{
}

WaveletMatrix::~WaveletMatrix()
{
}

void WaveletMatrix::Clear()
{
    vector<wat_array::BitArray>().swap(bit_arrays_);
    alphabet_num_ = 0;
    alphabet_bit_num_ = 0;
    length_ = 0;
}

void WaveletMatrix::Init(const vector<uint64_t>& array)
{
    Clear();
    alphabet_num_     = GetAlphabetNum(array);
    alphabet_bit_num_ = Log2(alphabet_num_);

    length_           = static_cast<uint64_t>(array.size());
    SetArray(array);
}

uint64_t WaveletMatrix::Lookup(uint64_t pos) const
{
    if (pos >= length_) return NOTFOUND;

    uint64_t index = pos;
    uint64_t c = 0;

    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        const wat_array::BitArray& ba = bit_arrays_[i];
        uint64_t bit = ba.Lookup(index);
        c <<= 1;
        c |= bit;
        index = ba.Rank(bit, index);
        if (bit)
        {
            index += zero_counts_[i];
        }
    }
    return c;
}

uint64_t WaveletMatrix::Rank(uint64_t c, uint64_t pos) const
{
    if (c >= alphabet_num_ || pos > length_)
    {
        return NOTFOUND;
    }
    uint64_t begin_pos = node_begin_pos_[alphabet_bit_num_ - 1]
                         [bit_reverse_table_[c]];
    uint64_t end_pos = pos;

    for (size_t i = 0; i < alphabet_bit_num_; ++i)
    {
        const wat_array::BitArray& ba = bit_arrays_[i];
        unsigned int bit = (c >> (alphabet_bit_num_ - i - 1)) & 1;
        end_pos = ba.Rank(bit, end_pos);
        if (bit)
        {
            end_pos += zero_counts_[i];
        }
    }
    return end_pos - begin_pos;
}

uint64_t WaveletMatrix::RankLessThan(uint64_t c, uint64_t pos) const
{
    uint64_t rank_less_than = 0;
    uint64_t rank_more_than = 0;
    uint64_t rank           = 0;
    RankAll(c, 0, pos, rank, rank_less_than, rank_more_than);
    return rank_less_than;
}

uint64_t WaveletMatrix::RankMoreThan(uint64_t c, uint64_t pos) const
{
    uint64_t rank_less_than = 0;
    uint64_t rank_more_than = 0;
    uint64_t rank           = 0;
    RankAll(c, 0, pos, rank, rank_less_than, rank_more_than);
    return rank_more_than;
}

void WaveletMatrix::RankAll(uint64_t c, uint64_t begin_pos, uint64_t end_pos,
                            uint64_t& rank,
                            uint64_t& rank_less_than,
                            uint64_t& rank_more_than) const
{
    if (c >= alphabet_num_ || begin_pos >= length_ || end_pos > length_)
    {
        rank_less_than = NOTFOUND;
        rank_more_than = NOTFOUND;
        rank           = NOTFOUND;
        return;
    }
    rank_less_than = 0;
    rank_more_than = 0;
    rank = 0;

    if (begin_pos >= end_pos)
    {
        return;
    }

    uint64_t more_and_less[2] = {0};
    uint64_t node_num = 0;
    bool from_zero = (begin_pos == 0);
    bool to_end = (end_pos == length_);

    for (size_t i = 0; i < alphabet_bit_num_; ++i)
    {
        const wat_array::BitArray& ba = bit_arrays_[i];
        unsigned int bit = (c >> (alphabet_bit_num_ - i - 1)) & 1;
        uint64_t range_bits = end_pos - begin_pos;
        uint64_t begin_zero, end_zero;

        if (from_zero)
        {
            begin_zero = node_begin_pos_[i][node_num];
        }
        else
        {
            begin_zero = ba.Rank(0, begin_pos);
        }

        if (to_end)
        {
            end_zero = node_begin_pos_[i][node_num+1];
        }
        else
        {
            end_zero = ba.Rank(0, end_pos);
        }

        if (bit)
        {
            begin_pos += zero_counts_[i] - begin_zero;
            end_pos += zero_counts_[i] - end_zero;
        }
        else
        {
            begin_pos = begin_zero;
            end_pos = end_zero;
        }
        more_and_less[bit] += range_bits - (end_pos - begin_pos);
        node_num |= bit << i;
    }
    rank_less_than = more_and_less[1];
    rank_more_than = more_and_less[0];
    rank = end_pos - begin_pos;
}


uint64_t WaveletMatrix::Select(uint64_t c, uint64_t rank) const
{
    return SelectFromPos(c, 0, rank);
}

uint64_t WaveletMatrix::SelectFromPos(uint64_t c,
                                      uint64_t pos,
                                      uint64_t rank) const
{
    if (c >= alphabet_num_)
    {
        return NOTFOUND;
    }
    if (pos >= length_)
    {
        return NOTFOUND;
    }

    uint64_t index;
    if (pos == 0)
    {
        index = node_begin_pos_[alphabet_bit_num_ - 1][bit_reverse_table_[c]];
    }
    else
    {
        index = pos;
        for (uint64_t i = 0; i < alphabet_bit_num_; ++i)
        {
            unsigned int bit = (c >> (alphabet_bit_num_ - i - 1)) & 1;
            index = bit_arrays_[i].Rank(bit, index);
            if (bit)
            {
                index += zero_counts_[i];
            }
        }
    }

    index += rank;

    for (int i = alphabet_bit_num_ - 1; i >= 0; --i)
    {
        unsigned int bit = (c >> (alphabet_bit_num_ - i - 1)) & 1;
        if (bit)
        {
            index -= zero_counts_[i];
        }

        index = bit_arrays_[i].Select(bit, index) + 1;

        if (index == wat_array::NOTFOUND)
        {
            return NOTFOUND;
        }
    }
    return index;
}

uint64_t WaveletMatrix::FreqRange(uint64_t min_c, uint64_t max_c,
                                  uint64_t begin_pos, uint64_t end_pos) const
{
    if (min_c >= alphabet_num_) return 0;
    if (max_c <= min_c) return 0;
    if (end_pos > length_ || begin_pos >= end_pos) return 0;
    uint64_t rank, max_less, min_less, more;
    RankAll(max_c, begin_pos, end_pos, rank, max_less, more);
    RankAll(min_c, begin_pos, end_pos, rank, min_less, more);
    return max_less - min_less;
}

void WaveletMatrix::MaxRange(uint64_t begin_pos, uint64_t end_pos, uint64_t& pos, uint64_t& val) const
{
    QuantileRange(begin_pos, end_pos, end_pos - begin_pos - 1, pos, val);
}

void WaveletMatrix::MinRange(uint64_t begin_pos, uint64_t end_pos, uint64_t& pos, uint64_t& val) const
{
    QuantileRange(begin_pos, end_pos, 0,  pos, val);
}

void WaveletMatrix::QuantileRange(uint64_t begin_pos, uint64_t end_pos,
                                  uint64_t k,
                                  uint64_t& pos, uint64_t& val)const
{
    if ((end_pos > length_ || begin_pos >= end_pos) ||
            (k >= end_pos - begin_pos))
    {
        pos = NOTFOUND;
        val = NOTFOUND;
        return;
    }

    val = 0;

    uint64_t node_num = 0;
    uint64_t begin_zero, end_zero;
    bool from_zero = (begin_pos == 0);
    bool to_end = (end_pos == length_);

    for (size_t i = 0; i < alphabet_bit_num_; ++i)
    {
        const wat_array::BitArray& ba = bit_arrays_[i];

        if (from_zero)
        {
            begin_zero = node_begin_pos_[i][node_num];
        }
        else
        {
            begin_zero = ba.Rank(0, begin_pos);
        }

        if (to_end)
        {
            end_zero = node_begin_pos_[i][node_num+1];
        }
        else
        {
            end_zero = ba.Rank(0, end_pos);
        }

        uint64_t zero_bits = end_zero - begin_zero;
        unsigned int bit = (k < zero_bits) ? 0 : 1;

        if (bit)
        {
            k -= zero_bits;
            begin_pos += zero_counts_[i] - begin_zero;
            end_pos += zero_counts_[i] - end_zero;
        }
        else
        {
            begin_pos = begin_zero;
            end_pos = end_zero;
        }
        node_num |= bit << i;
        val <<= 1;
        val |= bit;
    }

    pos = Select(val, begin_pos + k -
                 node_begin_pos_[alphabet_bit_num_ - 1]
                 [bit_reverse_table_[val]] + 1) - 1;
}

uint64_t WaveletMatrix::Freq(uint64_t c) const
{
    return Rank(c, length_);
}

uint64_t WaveletMatrix::FreqSum(uint64_t min_c, uint64_t max_c) const
{
    uint64_t sum = 0;
    for (uint64_t i = min_c; i < max_c; ++i)
    {
        sum += Freq(i);
    }
    return sum;
}

uint64_t WaveletMatrix::alphabet_num() const
{
    return alphabet_num_;
}

uint64_t WaveletMatrix::length() const
{
    return length_;
}

uint64_t WaveletMatrix::GetAlphabetNum(const std::vector<uint64_t>& array) const
{
    uint64_t alphabet_num = 0;
    for (size_t i = 0; i < array.size(); ++i)
    {
        if (array[i] >= alphabet_num)
        {
            alphabet_num = array[i]+1;
        }
    }
    return alphabet_num;
}

uint64_t WaveletMatrix::Log2(uint64_t x) const
{
    if (x == 0) return 0;
    x--;
    uint64_t bit_num = 0;
    while (x >> bit_num)
    {
        ++bit_num;
    }
    return bit_num;
}

inline uint64_t get_reversed_first_bits(uint64_t num,
                                        uint64_t max_bits,
                                        uint64_t bit_num,
                                        const vector<uint64_t>&
                                        bit_reverse_table)
{
    return bit_reverse_table[num & ((1 << max_bits) -
                                    (1 << (max_bits - bit_num)))];
}

void WaveletMatrix::SetBitReverseTable()
{
    bit_reverse_table_.resize(1 << alphabet_bit_num_);
    bit_reverse_table_[0] = 0;
    for (uint64_t i = 0; i < alphabet_bit_num_; ++i)
    {
        uint64_t n = (1 << i);
        uint64_t m = (1 << (alphabet_bit_num_ - i - 1));
        for (uint64_t j = 0; j < n; ++j)
        {
            bit_reverse_table_[n + j] = bit_reverse_table_[j] + m;
        }
    }
}

void WaveletMatrix::SetArray(const vector<uint64_t>& array)
{
    if (alphabet_num_ == 0) return;
    bit_arrays_.resize(alphabet_bit_num_, length_);
    zero_counts_.resize(alphabet_bit_num_, length_);

    node_begin_pos_.resize(alphabet_bit_num_);
    SetBitReverseTable();

    std::vector<uint64_t> dummy;
    dummy.push_back(0);
    dummy.push_back(length_);
    std::vector<uint64_t>& prev_begin_pos = dummy;

    for (uint64_t i = 0; i < alphabet_bit_num_; ++i)
    {
        node_begin_pos_[i].resize((1 << (i+1)) + 1);

        for (uint64_t j = 0; j < length_; ++j)
        {
            int bit = (array[j] >> (alphabet_bit_num_ - i - 1)) & 1;
            uint64_t subscript = get_reversed_first_bits(array[j],
                                 alphabet_bit_num_,
                                 i, bit_reverse_table_);
            bit_arrays_[i].SetBit(bit, prev_begin_pos[subscript]++);
            ++node_begin_pos_[i][subscript + (bit << i) + 1];
        }
        for (uint64_t j = 0; j < ((uint64_t)1 << i); ++j)
        {
            prev_begin_pos[j+1] = prev_begin_pos[j];
        }
        prev_begin_pos[0] = 0;

        for (uint64_t j = 0; j < ((uint64_t)1 << (i+1)); ++j)
        {
            node_begin_pos_[i][j+1] += node_begin_pos_[i][j];
        }
        zero_counts_[i] = node_begin_pos_[i][1 << i];

        bit_arrays_[i].Build();
        prev_begin_pos = node_begin_pos_[i];
    }
}

void WaveletMatrix::Save(ostream& os) const
{
    os.write((const char*)(&alphabet_num_), sizeof(alphabet_num_));
    os.write((const char*)(&length_), sizeof(length_));
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        bit_arrays_[i].Save(os);
    }
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        for (size_t j = 0; j < (size_t)(1 << (i+1)) + 1; ++j)
        {
            os.write((const char*)(&node_begin_pos_[i][j]),
                     sizeof(node_begin_pos_[i][j]));
        }
    }
}

void WaveletMatrix::Load(istream& is)
{
    Clear();
    is.read((char*)(&alphabet_num_), sizeof(alphabet_num_));
    alphabet_bit_num_ = Log2(alphabet_num_);
    is.read((char*)(&length_), sizeof(length_));

    bit_arrays_.resize(alphabet_bit_num_);
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        bit_arrays_[i].Load(is);
        bit_arrays_[i].Build();
    }

    node_begin_pos_.resize(bit_arrays_.size());
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        node_begin_pos_[i].resize((1 << (i+1)) + 1);
        for (size_t j = 0; j < (size_t)(1 << (i+1)) + 1; ++j)
        {
            is.read((char*)(&node_begin_pos_[i][j]),
                    sizeof(node_begin_pos_[i][j]));
        }
    }
    zero_counts_.resize(bit_arrays_.size());
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        zero_counts_[i] = node_begin_pos_[i][1 << i];
    }
    SetBitReverseTable();
}

}
