#ifndef _FM_INDEX_WAVELET_TREE_BINARY_HPP
#define _FM_INDEX_WAVELET_TREE_BINARY_HPP

#include "wavelet_tree.hpp"
#include "wavelet_tree_node.hpp"
#include "utils.hpp"
#include <am/succinct/sdarray/SDArray.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class WaveletTreeBinary : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;

    WaveletTreeBinary(size_t alphabet_num);
    ~WaveletTreeBinary();

    void build(const char_type *char_seq, size_t len);

    char_type access(size_t pos) const;
    char_type access(size_t pos, size_t &rank) const;

    size_t rank(char_type c, size_t pos) const;
    size_t select(char_type c, size_t rank) const;

    void intersect(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t thres,
            std::vector<char_type> &results) const;

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
            size_t start,
            char_type symbol,
            std::vector<char_type> &results) const;

private:
    size_t alphabet_bit_num_;
    sdarray::SDArray occ_;
    std::vector<WaveletTreeNode *> nodes_;
};

template <class CharT>
WaveletTreeBinary<CharT>::WaveletTreeBinary(size_t alphabet_num)
    : WaveletTree<CharT>(alphabet_num)
    , alphabet_bit_num_()
{
}

template <class CharT>
WaveletTreeBinary<CharT>::~WaveletTreeBinary()
{
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    alphabet_bit_num_ = bits(this->alphabet_num_ - 1);

    std::vector<std::vector<size_t> > beg_poses(alphabet_bit_num_ + 1);
    beg_poses[0].resize(1);
    for (size_t i = 1; i < beg_poses.size(); ++i)
    {
        beg_poses[i].resize(((this->alphabet_num_ - 1) >> (alphabet_bit_num_ - i)) + 2);
    }
    for (size_t i = 0; i < len; ++i)
    {
        const char_type &c = char_seq[i];
        for (size_t j = 1; j < beg_poses.size(); ++j)
        {
            ++beg_poses[j][(c >> (alphabet_bit_num_ - j)) + 1];
        }
    }
    for (size_t i = 1; i < beg_poses.size() - 1; ++i)
    {
        std::vector<size_t>& beg_poses_level = beg_poses[i];
        for (size_t j = 2; j < beg_poses_level.size(); ++j)
        {
            beg_poses_level[j] += beg_poses_level[j - 1];
        }
    }

    for (size_t i = 0; i < this->alphabet_num_; ++i)
    {
        occ_.add(beg_poses.back()[i + 1]);
    }
    beg_poses.pop_back();
    occ_.build();

    nodes_.resize(alphabet_bit_num_);
    for (size_t i = 0; i < alphabet_bit_num_; ++i)
    {
        nodes_[i] = new WaveletTreeNode;
        nodes_[i]->resize(len);
    }
    for (size_t i = 0; i < len; ++i)
    {
        const char_type &c = char_seq[i];
        nodes_[0]->changeBit(c >> (alphabet_bit_num_ - 1) & (char_type)1, beg_poses[0][0]++);
        for (size_t j = 1; j < beg_poses.size(); ++j)
        {
            nodes_[j]->changeBit(c >> (alphabet_bit_num_ - j - 1) & (char_type)1,
                                beg_poses[j][c >> (alphabet_bit_num_ - j)]++);
        }
    }

    std::vector<std::vector<size_t> >().swap(beg_poses);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->build();
    }
}

template <class CharT>
CharT WaveletTreeBinary<CharT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    size_t start = 0;
    size_t optR, before;

    char_type c = 0;
    char_type bit_mask = (char_type)1 << (alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        before = bv.Rank1(start);

        if (bv.GetBit(pos, optR))
        {
            c |= bit_mask;
            start = occ_.prefixSum(c);
            pos = start - before + optR;
        }
        else
        {
            pos = before + optR;
        }

        bit_mask >>= 1;
    }

    return c;
}

template <class CharT>
CharT WaveletTreeBinary<CharT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    size_t start = 0;
    size_t optR, before;

    char_type c = 0;
    char_type bit_mask = (char_type)1 << (alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        before = bv.Rank1(start);

        if (bv.GetBit(pos, optR))
        {
            c |= bit_mask;
            start = occ_.prefixSum(c);
            pos = start - before + optR;
        }
        else
        {
            pos = before + optR;
        }

        bit_mask >>= 1;
    }

    rank = pos - start;

    return c;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::rank(char_type c, size_t pos) const
{
    size_t start = 0;
    size_t rank = 0;
    size_t before;

    pos = std::min(pos, length());

    char_type masked = 0;
    char_type bit_mask = (char_type)1 << (alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        before = bv.Rank1(start);

        if (c & bit_mask)
        {
            masked |= bit_mask;
            start = occ_.prefixSum(masked);
            rank = bv.Rank1(pos) - before;
            pos = start + rank;
        }
        else
        {
            pos = pos - bv.Rank1(pos) + before;
            rank = pos - start;
        }

        if (rank == 0) return 0;

        bit_mask >>= 1;
    }

    return rank;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::select(char_type c, size_t rank) const
{
    size_t start, ones_start;

    char_type mask = ((char_type)1 << alphabet_bit_num_) - 2;
    char_type bit_mask = 1;

    for (size_t i = nodes_.size() - 1; i < nodes_.size(); --i)
    {
        const rsdic::RSDic &bv = nodes_[i]->bit_vector_;

        start = occ_.prefixSum(c & mask);
        ones_start = bv.Rank1(start);

        if (c & bit_mask)
        {
            rank = bv.Select1(ones_start + rank) - start;
        }
        else
        {
            rank = bv.Select0(start - ones_start + rank) - start;
        }

        mask <<= 1;
        bit_mask <<= 1;
    }

    return rank;
}

template <class CharT>
void WaveletTreeBinary<CharT>::intersect(
        const std::vector<std::pair<size_t, size_t> > &ranges,
        size_t thres,
        std::vector<char_type> &results) const
{
    if (thres > ranges.size()) return;
    if (thres > 0) thres = ranges.size() - thres;

    doIntersect_(ranges, thres, 0, 0, 0, results);
}

template <class CharT>
void WaveletTreeBinary<CharT>::doIntersect_(
        const std::vector<std::pair<size_t, size_t> > &ranges,
        size_t thres,
        size_t level,
        size_t start,
        char_type symbol,
        std::vector<char_type> &results) const
{
    if (level == nodes_.size())
    {
        results.push_back(symbol);
        return;
    }

    std::vector<std::pair<size_t, size_t> > sub_ranges;
    sub_ranges.reserve(ranges.size());

    const rsdic::RSDic &bv = nodes_[level]->bit_vector_;

    size_t before = bv.Rank0(start);
    size_t new_thres = thres;
    size_t rank_start, rank_end;

    for (size_t i = 0; i < ranges.size(); i++)
    {
        rank_start = bv.Rank0(start + ranges[i].first);
        rank_end = bv.Rank0(start + ranges[i].second);

        if (rank_start >= rank_end)
        {
            if (new_thres == 0) goto PRUNED_ZERO;
            else --new_thres;
        }
        else
        {
            sub_ranges.push_back(std::make_pair(rank_start - before, rank_end - before));
        }
    }

    doIntersect_(sub_ranges, new_thres, level + 1, start, symbol, results);

PRUNED_ZERO:
    sub_ranges.clear();
    before = bv.Rank1(start);
    new_thres = thres;

    for (size_t i = 0; i < ranges.size(); i++)
    {
        rank_start = bv.Rank1(start + ranges[i].first);
        rank_end = bv.Rank1(start + ranges[i].second);

        if (rank_start >= rank_end)
        {
            if (new_thres == 0) goto PRUNED_ONE;
            else --new_thres;
        }
        else
        {
            sub_ranges.push_back(std::make_pair(rank_start - before, rank_end - before));
        }
    }

    symbol |= (char_type)1 << level;
    start = occ_.prefixSum(symbol);

    doIntersect_(sub_ranges, new_thres, level + 1, start, symbol, results);

PRUNED_ONE:
    assert(true);
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::getOcc(char_type c) const
{
    return occ_.prefixSum(c);
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::length() const
{
    return nodes_.empty() ? 0 : nodes_[0]->length();
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::allocSize() const
{
    size_t sum = sizeof(WaveletTreeBinary<char_type>) + occ_.allocSize();
    for (size_t i = 0; i < nodes_.size(); ++i)
        sum += nodes_[i]->allocSize();

    return sum;
}

template <class CharT>
void WaveletTreeBinary<CharT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);
    occ_.save(ostr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->save(ostr);
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);
    occ_.load(istr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }

    alphabet_bit_num_ = bits(this->alphabet_num_ - 1);
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
