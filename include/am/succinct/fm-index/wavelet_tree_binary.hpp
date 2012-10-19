#ifndef _FM_INDEX_WAVELET_TREE_BINARY_HPP
#define _FM_INDEX_WAVELET_TREE_BINARY_HPP

#include "wavelet_tree.hpp"
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

    void topKUnion(
            const std::vector<boost::tuple<size_t, size_t, double> > &ranges,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results) const;

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

    for (size_t i = 1; i <= this->alphabet_num_; ++i)
    {
        occ_.add(beg_poses.back()[i]);
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
    for (size_t i = 1; i < nodes_.size(); ++i)
    {
        nodes_[i]->parent_ = nodes_[i - 1];
        nodes_[i - 1]->left_ = nodes_[i];
        nodes_[i - 1]->right_ = nodes_[i];
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

        if (rank == (size_t)-1) return -1;

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

    size_t before = bv.Rank1(start);
    size_t rank_start, rank_end;

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = ranges.begin();
            it != ranges.end(); ++it)
    {
        rank_start = bv.Rank1(start + it->first);
        rank_end = bv.Rank1(start + it->second);

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
                zero_ranges.push_back(std::make_pair(it->first - rank_start + before, it->second - rank_end + before));
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
                one_ranges.push_back(std::make_pair(rank_start - before, rank_end - before));
            }
        }
    }

    if (has_zeros)
    {
        doIntersect_(zero_ranges, zero_thres, level + 1, start, symbol, results);
    }

    if (has_ones)
    {
        symbol |= (char_type)1 << (alphabet_bit_num_ - level - 1);
        start = occ_.prefixSum(symbol);

        doIntersect_(one_ranges, one_thres, level + 1, start, symbol, results);
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::topKUnion(
        const std::vector<boost::tuple<size_t, size_t, double> > &ranges,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results) const
{
    if (topK == 0) return;

    boost::container::priority_deque<std::pair<RangeList *, size_t> > ranges_queue;
    size_t max_queue_size = std::max(topK, (size_t)1000);
    ranges_queue.push(std::make_pair(new RangeList((char_type)0, nodes_[0], ranges), 0));

    RangeList *top_ranges, *zero_ranges, *one_ranges;
    size_t start, before, rank_start, rank_end;

    while (!ranges_queue.empty() && results.size() < topK)
    {
        top_ranges = ranges_queue.top().first;
        start = ranges_queue.top().second;
        ranges_queue.pop_top();

        const WaveletTreeNode *node = top_ranges->node_;

        zero_ranges = new RangeList(top_ranges->sym_ << 1, node->left_);
        one_ranges = new RangeList(top_ranges->sym_ << 1 | (char_type)1, node->right_);

        before = node->bit_vector_.Rank1(start);

        for (std::vector<boost::tuple<size_t, size_t, double> >::const_iterator it = top_ranges->ranges_.begin();
                it != top_ranges->ranges_.end(); ++it)
        {
            if (it->get<0>() == it->get<1>())
            {
                zero_ranges->addRange(*it);
                one_ranges->addRange(*it);
            }
            else
            {
                rank_start = node->bit_vector_.Rank1(start + it->get<0>());
                rank_end = node->bit_vector_.Rank1(start + it->get<1>());

                zero_ranges->addRange(boost::make_tuple(it->get<0>() - rank_start + before, it->get<1>() - rank_end + before, it->get<2>()));
                one_ranges->addRange(boost::make_tuple(rank_start - before, rank_end - before, it->get<2>()));
            }
        }

        delete top_ranges;

        if (zero_ranges->score_ == 0.0)
        {
            delete zero_ranges;
        }
        else if (zero_ranges->node_)
        {
            ranges_queue.push(std::make_pair(zero_ranges, start));
        }
        else
        {
            results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
            delete zero_ranges;
        }

        if (one_ranges->score_ == 0.0)
        {
            delete one_ranges;
        }
        else if (one_ranges->node_)
        {
            ranges_queue.push(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));
        }
        else
        {
            results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
            delete one_ranges;
        }

        if (ranges_queue.size() > max_queue_size)
        {
            ranges_queue.pop_bottom();
        }
    }
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
    for (size_t i = 1; i < nodes_.size(); ++i)
    {
        nodes_[i]->parent_ = nodes_[i - 1];
        nodes_[i - 1]->left_ = nodes_[i];
        nodes_[i - 1]->right_ = nodes_[i];
    }
}

}
}

NS_IZENELIB_AM_END

#endif
