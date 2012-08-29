#ifndef _FM_INDEX_WAVELET_TREE_BINARY_H
#define _FM_INDEX_WAVELET_TREE_BINARY_H

#include "wavelet_tree.hpp"
#include "wavelet_tree_node.hpp"


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

    size_t length() const;
    size_t getSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    size_t alphabet_bit_num_;
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
        delete nodes_[i];
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
    for (size_t i = 1; i < beg_poses.size(); ++i)
    {
        std::vector<size_t>& beg_poses_level = beg_poses[i];
        for (size_t j = 2; j < beg_poses_level.size(); ++j)
        {
            beg_poses_level[j] += beg_poses_level[j - 1];
        }
    }

    this->occ_.swap(beg_poses.back());
    beg_poses.pop_back();

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
        const RRRBitVector &bv = nodes_[i]->bit_vector_;

        before = bv.rank1(start - 1);

        if (bv.access(pos, optR))
        {
            c |= bit_mask;
            start = this->occ_[c];
            pos = start - before + optR - 1;
        }
        else
        {
            pos = before + optR - 1;
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
        const RRRBitVector &bv = nodes_[i]->bit_vector_;

        before = bv.rank1(start - 1);

        if (bv.access(pos, optR))
        {
            c |= bit_mask;
            start = this->occ_[c];
            pos = start - before + optR - 1;
        }
        else
        {
            pos = before + optR - 1;
        }

        bit_mask >>= 1;
    }

    rank = pos - start + 1;

    return c;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::rank(char_type c, size_t pos) const
{
    size_t start = 0;
    size_t rank = 0;
    size_t before;

    char_type masked = 0;
    char_type bit_mask = (char_type)1 << (alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const RRRBitVector &bv = nodes_[i]->bit_vector_;

        before = bv.rank1(start - 1);

        if (c & bit_mask)
        {
            masked |= bit_mask;
            start = this->occ_[masked];
            rank = bv.rank1(pos) - before;
            pos = start + rank - 1;
        }
        else
        {
            pos = pos - bv.rank1(pos) + before;
            rank = pos - start + 1;
        }

        if (rank == 0) return 0;

        bit_mask >>= 1;
    }

    return rank;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::select(char_type c, size_t rank) const
{
    size_t pos = rank - 1;
    size_t start, ones_start;

    char_type mask = ((char_type)1 << alphabet_bit_num_) - 2;
    char_type bit_mask = 1;

    for (size_t i = nodes_.size() - 1; i < nodes_.size(); --i)
    {
        const RRRBitVector &bv = nodes_[i]->bit_vector_;

        start = this->occ_[c & mask];
        ones_start = bv.rank1(start - 1);

        if (c & bit_mask)
        {
            pos = bv.select1(ones_start + pos + 1) - start;
        }
        else
        {
            pos = bv.select0(start - ones_start + pos + 1) - start;
        }

        mask <<= 1;
        bit_mask <<= 1;
    }

    return pos;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::length() const
{
    return nodes_.empty() ? 0 : nodes_[0]->length();
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::getSize() const
{
    size_t sum = sizeof(WaveletTreeBinary<char_type>) + sizeof(this->occ_[0]) * this->occ_.size();
    for (size_t i = 0; i < nodes_.size(); ++i)
        sum += nodes_[i]->getSize();

    return sum;
}

template <class CharT>
void WaveletTreeBinary<CharT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->save(ostr);
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);

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
