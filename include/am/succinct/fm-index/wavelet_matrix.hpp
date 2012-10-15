#ifndef _FM_INDEX_WAVELET_MATRIX_HPP
#define _FM_INDEX_WAVELET_MATRIX_HPP

#include "wavelet_tree.hpp"
#include "utils.hpp"
#include <am/succinct/sdarray/SDArray.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class WaveletMatrix : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;

    WaveletMatrix(size_t alphabet_num);
    ~WaveletMatrix();

    void build(const char_type *char_seq, size_t len);

    char_type access(size_t pos) const;
    char_type access(size_t pos, size_t &rank) const;

    size_t rank(char_type c, size_t pos) const;
    size_t select(char_type c, size_t rank) const;

    void intersect(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t thres,
            std::vector<char_type> &results) const;

    void topKUnion(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t topK,
            std::vector<char_type> &result) const;

    size_t getOcc(char_type c) const;

    size_t length() const;
    size_t allocSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    void doIntersect_(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t thres,
            size_t level,
            char_type symbol,
            std::vector<char_type> &results) const;

private:
    size_t alphabet_bit_num_;
    size_t leading_bit_num_;
    sdarray::SDArray occ_;
    std::vector<size_t> zero_counts_;
    std::vector<WaveletTreeNode *> nodes_;
};

template <class CharT>
WaveletMatrix<CharT>::WaveletMatrix(size_t alphabet_num)
    : WaveletTree<CharT>(alphabet_num)
    , alphabet_bit_num_()
    , leading_bit_num_()
{
}

template <class CharT>
WaveletMatrix<CharT>::~WaveletMatrix()
{
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }
}

template <class CharT>
void WaveletMatrix<CharT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    alphabet_bit_num_ = bits(this->alphabet_num_ - 1);
    leading_bit_num_ = 64 - alphabet_bit_num_;

    zero_counts_.resize(alphabet_bit_num_);

    nodes_.resize(alphabet_bit_num_);

    std::vector<uint64_t> prev_begin_pos(1), node_begin_pos;
    char_type bit_mask, subscript;

    for (size_t i = 0; i < alphabet_bit_num_; ++i)
    {
        node_begin_pos.clear();
        node_begin_pos.resize((1ULL << (i + 1)) + 1);

        nodes_[i] = new WaveletTreeNode;
        nodes_[i]->resize(len);

        bit_mask = (char_type)1 << i;

        for (size_t j = 0; j < len; ++j)
        {
            subscript = char_seq[j] & (bit_mask - 1);

            if (char_seq[j] & bit_mask)
            {
                nodes_[i]->setBit(prev_begin_pos[subscript]++);
                ++node_begin_pos[subscript + bit_mask + 1];
            }
            else
            {
                ++node_begin_pos[subscript + 1];
            }
        }

        nodes_[i]->build();

        prev_begin_pos = node_begin_pos;
        for (size_t j = 2; j < prev_begin_pos.size(); ++j)
        {
            prev_begin_pos[j] += prev_begin_pos[j - 1];
        }
        zero_counts_[i] = prev_begin_pos[bit_mask];
    }

    for (size_t i = 1; i <= this->alphabet_num_; ++i)
    {
        occ_.add(node_begin_pos[i]);
    }
    occ_.build();
}

template <class CharT>
CharT WaveletMatrix<CharT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    char_type c = 0;
    char_type bit_mask = 1;

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        if (bv.GetBit(pos, pos))
        {
            c |= bit_mask;
            pos += zero_counts_[i];
        }

        bit_mask <<= 1;
    }

    return c;
}

template <class CharT>
CharT WaveletMatrix<CharT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    char_type c = 0;
    char_type bit_mask = 1;

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        if (bv.GetBit(pos, pos))
        {
            c |= bit_mask;
            pos += zero_counts_[i];
        }

        bit_mask <<= 1;
    }

    rank = pos - occ_.prefixSum(c);

    return c;
}

template <class CharT>
size_t WaveletMatrix<CharT>::rank(char_type c, size_t pos) const
{
    pos = std::min(pos, length());

    char_type bit_mask = 1;

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        if (c & bit_mask)
        {
            pos = bv.Rank1(pos) + zero_counts_[i];
        }
        else
        {
            pos -= bv.Rank1(pos);
        }

        bit_mask <<= 1;
    }

    return pos - occ_.prefixSum(c);
}

template <class CharT>
size_t WaveletMatrix<CharT>::select(char_type c, size_t rank) const
{
    size_t pos = rank + occ_.prefixSum(c);
    char_type bit_mask = (char_type)1 << (alphabet_bit_num_ - 1);

    for (size_t i = nodes_.size() - 1; i < nodes_.size(); --i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        if (c & bit_mask)
        {
            pos = bv.Select1(pos - zero_counts_[i]);
        }
        else
        {
            pos = bv.Select0(pos);
        }

        if (pos == (size_t)-1) return -1;

        bit_mask >>= 1;
    }

    return pos;
}

template <class CharT>
void WaveletMatrix<CharT>::intersect(
        const std::vector<std::pair<size_t, size_t> > &ranges,
        size_t thres,
        std::vector<char_type> &results) const
{
    if (thres > ranges.size()) return;
    if (thres > 0) thres = ranges.size() - thres;

    doIntersect_(ranges, thres, 0, 0, results);
}

template <class CharT>
void WaveletMatrix<CharT>::doIntersect_(
        const std::vector<std::pair<size_t, size_t> > &ranges,
        size_t thres,
        size_t level,
        char_type symbol,
        std::vector<char_type> &results) const
{
    if (level == alphabet_bit_num_)
    {
        results.push_back(symbol);
        return;
    }

    std::vector<std::pair<size_t, size_t> > zero_ranges, one_ranges;
    zero_ranges.reserve(ranges.size());
    one_ranges.reserve(ranges.size());

    size_t zero_thres = thres, one_thres = thres;
    bool has_zeros = true, has_ones = true;

    const rsdic::RSDic &bv = nodes_[level]->bit_vector_;

    size_t rank_start, rank_end;

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = ranges.begin();
            it != ranges.end(); ++it)
    {
        rank_start = bv.Rank1(it->first);
        rank_end = bv.Rank1(it->second);

        if (has_zeros)
        {
            if (it->first - rank_start >= it->second - rank_end)
            {
                if (zero_thres == 0)
                {
                    if (!has_ones) return;
                    has_zeros = false;
                }
                else
                {
                    --zero_thres;
                }
            }
            else
            {
                zero_ranges.push_back(std::make_pair(it->first - rank_start, it->second - rank_end));
            }
        }

        if (has_ones)
        {
            if (rank_start >= rank_end)
            {
                if (one_thres == 0)
                {
                    if (!has_zeros) return;
                    has_ones = false;
                }
                else
                {
                    --one_thres;
                }
            }
            else
            {
                one_ranges.push_back(std::make_pair(rank_start + zero_counts_[level], rank_end + zero_counts_[level]));
            }
        }
    }

    if (has_zeros)
    {
        doIntersect_(zero_ranges, zero_thres, level + 1, symbol, results);
    }

    if (has_ones)
    {
        symbol |= (char_type)1 << level;

        doIntersect_(one_ranges, one_thres, level + 1, symbol, results);
    }
}

template <class CharT>
void WaveletMatrix<CharT>::topKUnion(
        const std::vector<std::pair<size_t, size_t> > &ranges,
        size_t topK,
        std::vector<char_type> &results) const
{
    if (topK == 0) return;
}

template <class CharT>
size_t WaveletMatrix<CharT>::getOcc(char_type c) const
{
    return occ_.prefixSum(c);
}

template <class CharT>
size_t WaveletMatrix<CharT>::length() const
{
    return nodes_.empty() ? 0 : nodes_[0]->length();
}

template <class CharT>
size_t WaveletMatrix<CharT>::allocSize() const
{
    size_t sum = sizeof(WaveletMatrix<char_type>) + occ_.allocSize() + sizeof(zero_counts_[0]) * zero_counts_.size();
    for (size_t i = 0; i < nodes_.size(); ++i)
        sum += nodes_[i]->allocSize();

    return sum;
}

template <class CharT>
void WaveletMatrix<CharT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);
    occ_.save(ostr);

    ostr.write((const char *)&zero_counts_[0], sizeof(zero_counts_[0]) * zero_counts_.size());

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->save(ostr);
    }
}

template <class CharT>
void WaveletMatrix<CharT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);
    occ_.load(istr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }

    alphabet_bit_num_ = bits(this->alphabet_num_ - 1);
    leading_bit_num_ = 64 - alphabet_bit_num_;

    zero_counts_.resize(alphabet_bit_num_);
    istr.read((char *)&zero_counts_[0], sizeof(zero_counts_[0]) * zero_counts_.size());

    nodes_.resize(alphabet_bit_num_);
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i] = new WaveletTreeNode;
        nodes_[i]->load(istr);
    }
}

}
}

NS_IZENELIB_AM_END

#endif
