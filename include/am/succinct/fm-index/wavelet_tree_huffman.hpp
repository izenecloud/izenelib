#ifndef _FM_INDEX_WAVELET_TREE_HUFFMAN_HPP
#define _FM_INDEX_WAVELET_TREE_HUFFMAN_HPP

#include "wavelet_tree.hpp"
#include "wavelet_tree_node.hpp"
#include <am/interval_heap.hpp>

#include <deque>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT, class BitmapT>
class WaveletTreeHuffman : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;
    typedef WaveletTreeHuffman<CharT, BitmapT> self_type;
    typedef WaveletTreeNode<BitmapT> node_type;

    WaveletTreeHuffman(uint64_t alphabet_num, bool support_select);
    ~WaveletTreeHuffman();

    void build(const char_type *char_seq, size_t len);

    char_type access(size_t pos) const;
    char_type access(size_t pos, size_t &rank) const;

    size_t rank(char_type c, size_t pos) const;
    size_t select(char_type c, size_t rank) const;

    void intersect(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &results) const;

    size_t beginOcc(char_type c) const;
    size_t endOcc(char_type c) const;

    //WaveletTreeNode *getRoot() const;

    size_t length() const;
    size_t allocSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    void makeCodeMap_(uint64_t code, size_t level, node_type *node);

    void deleteTree_(node_type *node);
    void buildTreeNodes_(node_type *node);

    void recursiveIntersect_(
            const node_type *node,
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &results) const;

    size_t getTreeSize_(const node_type *node) const;

    void saveTree_(std::ostream &ostr, const node_type *node) const;
    void loadTree_(std::istream &istr, node_type *node);

private:
    std::vector<size_t> occ_;
    std::vector<uint64_t> code_map_;
    node_type *root_;
    std::vector<node_type *> leaves_;
};

template <class CharT, class BitmapT>
WaveletTreeHuffman<CharT, BitmapT>::WaveletTreeHuffman(uint64_t alphabet_num, bool support_select)
    : WaveletTree<CharT>(alphabet_num, support_select)
    , root_()
{
}

template <class CharT, class BitmapT>
WaveletTreeHuffman<CharT, BitmapT>::~WaveletTreeHuffman()
{
    if (root_) deleteTree_(root_);
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::deleteTree_(node_type *node)
{
    if (node->left_) deleteTree_(node->left_);
    if (node->right_) deleteTree_(node->right_);
    delete node;
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    occ_.resize((1 << this->alphabet_bit_num_) + 1);
    for (size_t i = 0; i < len; ++i)
    {
        ++occ_[char_seq[i] + 1];
    }

    leaves_.resize(this->alphabet_num_);
    interval_heap<node_type *> node_queue(this->alphabet_num_);

    for (size_t i = 0; i < this->alphabet_num_; ++i)
    {
        if (occ_[i + 1])
        {
            leaves_[i] = new node_type(i, occ_[i + 1], this->support_select_);
            node_queue.insert((leaves_[i]));
        }
    }

    while (node_queue.size() > 1)
    {
        node_type *left = node_queue.get_min();
        node_queue.pop_min();

        node_type *right = node_queue.get_min();
        node_queue.pop_min();

        node_queue.insert(new node_type(left, right, this->support_select_));
    }

    root_ = node_queue.get_min();
    node_queue.pop_min();

    code_map_.resize(this->alphabet_num_);
    makeCodeMap_(0, 0, root_);

    for (size_t i = 0; i < this->alphabet_num_; ++i)
    {
        if (leaves_[i] && leaves_[i]->parent_)
        {
            if (leaves_[i]->parent_->left_ == leaves_[i])
            {
                leaves_[i]->parent_->c0_ = i;
                leaves_[i] = leaves_[i]->parent_;

                delete leaves_[i]->left_;
                leaves_[i]->left_ = NULL;
            }
            else
            {
                leaves_[i]->parent_->c1_ = i;
                leaves_[i] = leaves_[i]->parent_;

                delete leaves_[i]->right_;
                leaves_[i]->right_ = NULL;
            }
        }
    }

    for (size_t i = 2; i < occ_.size(); ++i)
    {
        occ_[i] += occ_[i - 1];
    }

    uint64_t code;
    size_t level;
    node_type *walk;

    for (size_t i = 0; i < len; ++i)
    {
        code = code_map_[char_seq[i]];
        walk = root_;

        for (level = 0; walk; ++level)
        {
            if (code & 1ULL << level)
            {
                walk->append1();
                walk = walk->right_;
            }
            else
            {
                walk->append0();
                walk = walk->left_;
            }
        }
    }

    buildTreeNodes_(root_);
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::makeCodeMap_(uint64_t code, size_t level, node_type *node)
{
    if (node->left_)
    {
        makeCodeMap_(code, level + 1, node->left_);
        makeCodeMap_(code | 1ULL << level, level + 1, node->right_);
    }
    else
    {
        code_map_[node->c0_] = code;
    }
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::buildTreeNodes_(node_type *node)
{
    if (!node) return;
    node->build();
    buildTreeNodes_(node->left_);
    buildTreeNodes_(node->right_);
}

template <class CharT, class BitmapT>
CharT WaveletTreeHuffman<CharT, BitmapT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    node_type *walk = root_;

    while (true)
    {
        if (walk->access(pos, pos))
        {
            if (walk->right_) walk = walk->right_;
            else return walk->c1_;
        }
        else
        {
            if (walk->left_) walk = walk->left_;
            else return walk->c0_;
        }
    }
}

template <class CharT, class BitmapT>
CharT WaveletTreeHuffman<CharT, BitmapT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    node_type *walk = root_;

    while (true)
    {
        if (walk->access(pos, pos))
        {
            if (walk->right_)
            {
                walk = walk->right_;
            }
            else
            {
                rank = pos;
                return walk->c1_;
            }
        }
        else
        {
            if (walk->left_)
            {
                walk = walk->left_;
            }
            else
            {
                rank = pos;
                return walk->c0_;
            }
        }
    }
}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::rank(char_type c, size_t pos) const
{
    if (c >= leaves_.size() || !leaves_[c]) return 0;

    pos = std::min(pos, length());

    uint64_t code = code_map_[c];
    node_type *walk = root_;

    for (size_t level = 0; pos > 0; ++level)
    {
        if (code & 1ULL << level)
        {
            if (walk->right_)
            {
                pos = walk->rank1(pos);
                walk = walk->right_;
            }
            else
            {
                return walk->rank1(pos);
            }
        }
        else
        {
            if (walk->left_)
            {
                pos = walk->rank0(pos);
                walk = walk->left_;
            }
            else
            {
                return walk->rank0(pos);
            }
        }
    }

    return 0;
}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::select(char_type c, size_t rank) const
{
    if (!leaves_[c]) return -1;

    node_type *walk = leaves_[c];
    if (!walk) return -1;

    bool bit = (walk->c1_ == c);

    for (; walk->parent_; walk = walk->parent_)
    {
        if ((rank = walk->select(rank, bit)) == (size_t)-1)
            return -1;

        bit = (walk == walk->parent_->right_);
    }

    return walk->select(rank, bit);
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::intersect(
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        std::vector<char_type> &results) const
{
    if (thres > patterns.size()) return;
    if (thres > 0) thres = patterns.size() - thres;

    results.reserve(max_count);
    recursiveIntersect_(root_, patterns, thres, max_count, results);
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::recursiveIntersect_(
        const node_type *node,
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        std::vector<char_type> &results) const
{
    if (results.size() >= max_count) return;

    std::vector<std::pair<size_t, size_t> > zero_ranges, one_ranges;
    zero_ranges.reserve(patterns.size());
    one_ranges.reserve(patterns.size());

    size_t zero_thres = thres, one_thres = thres;
    bool has_zeros = true, has_ones = true;

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = patterns.begin();
            it != patterns.end(); ++it)
    {
        size_t rank_start = node->rank1(it->first);
        size_t rank_end = node->rank1(it->second);

        if (has_zeros)
        {
            if (it->first - rank_start < it->second - rank_end)
            {
                zero_ranges.push_back(std::make_pair(it->first - rank_start, it->second - rank_end));
            }
            else if (zero_thres-- == 0)
            {
                if (!has_ones) return;
                has_zeros = false;
            }
        }

        if (has_ones)
        {
            if (rank_start < rank_end)
            {
                one_ranges.push_back(std::make_pair(rank_start, rank_end));
            }
            else if (one_thres-- == 0)
            {
                if (!has_zeros) return;
                has_ones = false;
            }
        }
    }

    if (has_zeros)
    {
        if (node->left_)
        {
            recursiveIntersect_(node->left_, zero_ranges, zero_thres, max_count, results);
        }
        else
        {
            results.push_back(node->c0_);
        }
    }

    if (results.size() < max_count && has_ones)
    {
        if (node->right_)
        {
            recursiveIntersect_(node->right_, one_ranges, one_thres, max_count, results);
        }
        else
        {
            results.push_back(node->c1_);
        }
    }
}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::beginOcc(char_type c) const
{
    if (c < occ_.size()) return occ_[c];
    return occ_.back();
}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::endOcc(char_type c) const
{
    if (c < occ_.size() - 1) return occ_[c + 1];
    return occ_.back();
}

//template <class CharT, class BitmapT>
//node_type *WaveletTreeHuffman<CharT, BitmapT>::getRoot() const
//{
//    return root_;
//}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::length() const
{
    return root_ ? root_->length() : 0;
}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::allocSize() const
{
    return sizeof(self_type)
        + sizeof(occ_[0]) * occ_.size()
        + sizeof(code_map_[0]) * code_map_.size()
        + getTreeSize_(root_);
}

template <class CharT, class BitmapT>
size_t WaveletTreeHuffman<CharT, BitmapT>::getTreeSize_(const node_type *node) const
{
    if (!node) return 0;
    return node->allocSize() + getTreeSize_(node->left_) + getTreeSize_(node->right_);
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);

    ostr.write((const char *)&occ_[0], sizeof(occ_[0]) * occ_.size());
    ostr.write((const char *)&code_map_[0], sizeof(code_map_[0]) * code_map_.size());

    if (root_)
    {
        uint32_t flag = 1U;
        ostr.write((const char *)&flag, sizeof(flag));
        saveTree_(ostr, root_);
    }
    else
    {
        uint32_t flag = 0U;
        ostr.write((const char *)&flag, sizeof(flag));
    }
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::saveTree_(std::ostream &ostr, const node_type *node) const
{
    node->save(ostr);

    uint32_t flag = (node->left_ ? 1U : 0U) | (node->right_ ? 2U : 0U);
    ostr.write((const char *)&flag, sizeof(flag));

    if (node->left_) saveTree_(ostr, node->left_);
    if (node->right_) saveTree_(ostr, node->right_);
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);

    if (root_) deleteTree_(root_);

    occ_.resize((1 << this->alphabet_bit_num_) + 1);
    istr.read((char *)&occ_[0], sizeof(occ_[0]) * occ_.size());
    code_map_.resize(this->alphabet_num_);
    istr.read((char *)&code_map_[0], sizeof(code_map_[0]) * code_map_.size());

    uint32_t flag = 0;
    istr.read((char *)&flag, sizeof(flag));
    if (flag)
    {
        root_ = new node_type(this->support_select_);
        leaves_.resize(this->alphabet_num_);
        loadTree_(istr, root_);
    }
}

template <class CharT, class BitmapT>
void WaveletTreeHuffman<CharT, BitmapT>::loadTree_(std::istream &istr, node_type *node)
{
    node->load(istr);

    uint32_t flag = 0;
    istr.read((char *)&flag, sizeof(flag));

    if (flag & 1U)
    {
        node->left_ = new node_type(this->support_select_);
        node->left_->parent_ = node;
        loadTree_(istr, node->left_);
    }
    else
    {
        leaves_[node->c0_] = node;
    }

    if (flag & 2U)
    {
        node->right_ = new node_type(this->support_select_);
        node->right_->parent_ = node;
        loadTree_(istr, node->right_);
    }
    else
    {
        leaves_[node->c1_] = node;
    }
}

}
}

NS_IZENELIB_AM_END

#endif
