#ifndef _FM_INDEX_WAVELET_TREE_HUFFMAN_HPP
#define _FM_INDEX_WAVELET_TREE_HUFFMAN_HPP

#include "wavelet_tree.hpp"
#include "wavelet_tree_node.hpp"

#include <queue>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class WaveletTreeHuffman : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;

    WaveletTreeHuffman(size_t alphabet_num);
    ~WaveletTreeHuffman();

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
    void makeCodeMap_(size_t code, size_t level, WaveletTreeNode *node);

    void deleteTree_(WaveletTreeNode *node);
    void buildTreeNodes_(WaveletTreeNode *node);

    size_t getTreeSize_(const WaveletTreeNode *node) const;

    void saveTree_(std::ostream &ostr, const WaveletTreeNode *node) const;
    void loadTree_(std::istream &istr, WaveletTreeNode *node);

private:
    std::vector<size_t> code_map_;
    WaveletTreeNode *root_;
    std::vector<WaveletTreeNode *> leaves_;
};

template <class CharT>
WaveletTreeHuffman<CharT>::WaveletTreeHuffman(size_t alphabet_num)
    : WaveletTree<CharT>(alphabet_num)
    , root_()
{
}

template <class CharT>
WaveletTreeHuffman<CharT>::~WaveletTreeHuffman()
{
    if (root_) deleteTree_(root_);
}

template <class CharT>
void WaveletTreeHuffman<CharT>::deleteTree_(WaveletTreeNode *node)
{
    if (node->left_) deleteTree_(node->left_);
    if (node->right_) deleteTree_(node->right_);
    delete node;
}

template <class CharT>
void WaveletTreeHuffman<CharT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    this->occ_.resize(this->alphabet_num_ + 1);
    for (size_t i = 0; i < len; ++i)
    {
        ++this->occ_[char_seq[i] + 1];
    }

    leaves_.resize(this->alphabet_num_);
    std::priority_queue<WaveletTreeNode *, std::vector<WaveletTreeNode *>, std::greater<WaveletTreeNode *> > node_queue;

    for (size_t i = 0; i < this->alphabet_num_; ++i)
    {
        if (this->occ_[i + 1])
        {
            leaves_[i] = new WaveletTreeNode(i, this->occ_[i + 1]);
            node_queue.push((leaves_[i]));
        }
    }

    while (node_queue.size() > 1)
    {
        WaveletTreeNode *left = node_queue.top();
        node_queue.pop();

        WaveletTreeNode *right = node_queue.top();
        node_queue.pop();

        node_queue.push(new WaveletTreeNode(left, right));
    }

    root_ = node_queue.top();
    node_queue.pop();

    code_map_.resize(this->alphabet_num_);
    makeCodeMap_(0, 0, root_);

    for (size_t i = 0; i < this->alphabet_num_; ++i)
    {
        if (leaves_[i])
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

    for (size_t i = 2; i < this->occ_.size(); ++i)
    {
        this->occ_[i] += this->occ_[i - 1];
    }

    size_t code, level;
    WaveletTreeNode *walk;

    for (size_t i = 0; i < len; ++i)
    {
        code = code_map_[char_seq[i]];
        walk = root_;

        for (level = 0;; ++level)
        {
            if (code & 1ULL << level)
            {
                walk->append1();

                walk = walk->right_;
                if (!walk) break;
            }
            else
            {
                walk->append0();

                walk = walk->left_;
                if (!walk) break;
            }
        }
    }

    buildTreeNodes_(root_);
}

template <class CharT>
void WaveletTreeHuffman<CharT>::makeCodeMap_(size_t code, size_t level, WaveletTreeNode *node)
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

template <class CharT>
void WaveletTreeHuffman<CharT>::buildTreeNodes_(WaveletTreeNode *node)
{
    if (!node) return;
    node->build();
    buildTreeNodes_(node->left_);
    buildTreeNodes_(node->right_);
}

template <class CharT>
CharT WaveletTreeHuffman<CharT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    WaveletTreeNode *walk = root_;

    while (true)
    {
        if (walk->bit_vector_.GetBit(pos, pos))
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

template <class CharT>
CharT WaveletTreeHuffman<CharT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    WaveletTreeNode *walk = root_;

    while (true)
    {
        if (walk->bit_vector_.GetBit(pos, pos))
        {
            if (walk->right_)
            {
                walk = walk->right_;
            }
            else
            {
                rank = pos + 1;
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
                rank = pos + 1;
                return walk->c0_;
            }
        }
    }
}

template <class CharT>
size_t WaveletTreeHuffman<CharT>::rank(char_type c, size_t pos) const
{
    if (!leaves_[c]) return 0;

    pos = std::min(pos + 1, length());

    size_t code = code_map_[c];
    WaveletTreeNode *walk = root_;

    for (size_t level = 0; pos > 0; ++level)
    {
        if (code & 1U << level)
        {
            if (walk->right_)
            {
                pos = walk->bit_vector_.Rank1(pos);
                walk = walk->right_;
            }
            else
            {
                return walk->bit_vector_.Rank1(pos);
            }
        }
        else
        {
            if (walk->left_)
            {
                pos = walk->bit_vector_.Rank0(pos);
                walk = walk->left_;
            }
            else
            {
                return walk->bit_vector_.Rank0(pos);
            }
        }
    }

    return 0;
}

template <class CharT>
size_t WaveletTreeHuffman<CharT>::select(char_type c, size_t rank) const
{
    if (!leaves_[c]) return -1;

    WaveletTreeNode *walk = leaves_[c];
    if (!walk) return -1;

    bool bit = (walk->c1_ == c);
    --rank;

    for (; walk->parent_; walk = walk->parent_)
    {
        if ((rank = walk->bit_vector_.Select(rank, bit)) == -1)
            return -1;

        bit = (walk == walk->parent_->right_);
    }

    return walk->bit_vector_.Select(rank, bit);
}

template <class CharT>
size_t WaveletTreeHuffman<CharT>::length() const
{
    return root_ ? root_->length() : 0;
}

template <class CharT>
size_t WaveletTreeHuffman<CharT>::getSize() const
{
    return sizeof(WaveletTreeHuffman<char_type>)
        + sizeof(this->occ_[0]) * this->occ_.size()
        + sizeof(code_map_[0]) * code_map_.size()
        + getTreeSize_(root_);
}

template <class CharT>
size_t WaveletTreeHuffman<CharT>::getTreeSize_(const WaveletTreeNode *node) const
{
    if (!node) return 0;
    return node->getSize() + getTreeSize_(node->left_) + getTreeSize_(node->right_);
}

template <class CharT>
void WaveletTreeHuffman<CharT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);

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

template <class CharT>
void WaveletTreeHuffman<CharT>::saveTree_(std::ostream &ostr, const WaveletTreeNode *node) const
{
    node->save(ostr);

    uint32_t flag = (node->left_ ? 1U : 0U) | (node->right_ ? 2U : 0U);
    ostr.write((const char *)&flag, sizeof(flag));

    if (node->left_) saveTree_(ostr, node->left_);
    if (node->right_) saveTree_(ostr, node->right_);
}

template <class CharT>
void WaveletTreeHuffman<CharT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);

    if (root_) deleteTree_(root_);

    code_map_.resize(this->alphabet_num_);
    istr.read((char *)&code_map_[0], sizeof(code_map_[0]) * code_map_.size());

    uint32_t flag = 0;
    istr.read((char *)&flag, sizeof(flag));
    if (flag)
    {
        root_ = new WaveletTreeNode;
        leaves_.resize(this->alphabet_num_);
        loadTree_(istr, root_);
    }
}

template <class CharT>
void WaveletTreeHuffman<CharT>::loadTree_(std::istream &istr, WaveletTreeNode *node)
{
    node->load(istr);

    uint32_t flag = 0;
    istr.read((char *)&flag, sizeof(flag));

    if (flag)
    {
        if (flag & 1U)
        {
            node->left_ = new WaveletTreeNode;
            node->left_->parent_ = node;
            loadTree_(istr, node->left_);
        }
        if (flag & 2U)
        {
            node->right_ = new WaveletTreeNode;
            node->right_->parent_ = node;
            loadTree_(istr, node->right_);
        }
    }
    else
    {
        leaves_[node->c0_] = node;
        leaves_[node->c1_] = node;
    }
}

}
}

NS_IZENELIB_AM_END

#endif
