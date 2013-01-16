#include <am/succinct/fm-index/wavelet_tree_node.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

WaveletTreeNode::WaveletTreeNode(bool support_select, bool dense)
    : left_(), right_(), parent_()
    , c0_(), c1_()
    , bit_vector_(support_select)
    , dense_bit_vector_(support_select)
    , dense_(dense)
    , freq_(), len_()
{
}

WaveletTreeNode::WaveletTreeNode(uint64_t c, size_t freq, bool support_select, bool dense)
    : left_(), right_(), parent_()
    , c0_(c), c1_()
    , bit_vector_(support_select)
    , dense_bit_vector_(support_select)
    , dense_(dense)
    , freq_(freq), len_()
{
}

WaveletTreeNode::WaveletTreeNode(WaveletTreeNode *left, WaveletTreeNode *right, bool support_select, bool dense)
    : left_(left), right_(right), parent_()
    , c0_(), c1_()
    , bit_vector_(support_select)
    , dense_bit_vector_(support_select)
    , dense_(dense)
    , freq_(left->freq_ + right->freq_), len_()
{
    left->parent_ = this;
    right->parent_ = this;
}

WaveletTreeNode::~WaveletTreeNode()
{
}

void WaveletTreeNode::resize(size_t len)
{
    len_ = len;
    size_t index = len_ / 64;
    if (index >= raw_array_.size())
        raw_array_.resize(index + 1);
}

void WaveletTreeNode::setBit(size_t pos)
{
    size_t index = pos / 64, offset = pos % 64;
    if (pos >= len_)
    {
        len_ = pos + 1;
        if (index >= raw_array_.size())
            raw_array_.resize(index + 1);
    }
    raw_array_[index] |= 1LLU << offset;
}

void WaveletTreeNode::unsetBit(size_t pos)
{
    size_t index = pos / 64/*, offset = pos % 64*/;
    if (pos >= len_)
    {
        len_ = pos + 1;
        if (index >= raw_array_.size())
            raw_array_.resize(index + 1);
    }
//  raw_array_[index] &= ~(1LLU << offset);
}

void WaveletTreeNode::changeBit(bool bit, size_t pos)
{
    if (bit) setBit(pos);
    else unsetBit(pos);
}

void WaveletTreeNode::append0()
{
    size_t index = len_ / 64/*, offset = len_ % 64*/;
    if (index >= raw_array_.size())
        raw_array_.resize(index + 1);
//  raw_array_[index] &= ~(1LLU << offset);
    ++len_;
}

void WaveletTreeNode::append1()
{
    size_t index = len_ / 64, offset = len_ % 64;
    if (index >= raw_array_.size())
        raw_array_.resize(index + 1);
    raw_array_[index] |= 1LLU << offset;
    ++len_;
}

void WaveletTreeNode::append(bool bit)
{
    if (bit) append1();
    else append0();
}

void WaveletTreeNode::build()
{
    if (!raw_array_.empty())
    {
        if (dense_)
        {
            dense_bit_vector_.build(raw_array_, len_);
        }
        else
        {
            bit_vector_.Build(raw_array_, len_);
        }
        std::vector<uint64_t>().swap(raw_array_);
    }
}

bool WaveletTreeNode::access(size_t pos) const
{
    return dense_ ? dense_bit_vector_.lookup(pos) : bit_vector_.GetBit(pos);
}

bool WaveletTreeNode::access(size_t pos, size_t& rank) const
{
    return dense_ ? dense_bit_vector_.lookup(pos, rank) : bit_vector_.GetBit(pos, rank);
}

size_t WaveletTreeNode::rank0(size_t pos) const
{
    return dense_ ? dense_bit_vector_.rank0(pos) : bit_vector_.Rank0(pos);
}

size_t WaveletTreeNode::rank1(size_t pos) const
{
    return dense_ ? dense_bit_vector_.rank1(pos) : bit_vector_.Rank1(pos);
}

size_t WaveletTreeNode::rank(size_t pos, bool bit) const
{
    return dense_ ? dense_bit_vector_.rank(pos, bit) : bit_vector_.Rank(pos, bit);
}

size_t WaveletTreeNode::select0(size_t ind) const
{
    return dense_ ? dense_bit_vector_.select0(ind) : bit_vector_.Select0(ind);
}

size_t WaveletTreeNode::select1(size_t ind) const
{
    return dense_ ? dense_bit_vector_.select1(ind) : bit_vector_.Select1(ind);
}

size_t WaveletTreeNode::select(size_t ind, bool bit) const
{
    return dense_ ? dense_bit_vector_.select(ind, bit) : bit_vector_.Select(ind, bit);
}

size_t WaveletTreeNode::length() const
{
    return dense_ ? dense_bit_vector_.length() : bit_vector_.num();
}

size_t WaveletTreeNode::allocSize() const
{
    return sizeof(WaveletTreeNode)
        + bit_vector_.GetUsageBytes() - sizeof(rsdic::RSDic)
        + dense_bit_vector_.allocSize() - sizeof(dense::DBitV)
        + sizeof(raw_array_[0]) * raw_array_.size();
}

void WaveletTreeNode::save(std::ostream &ostr) const
{
    ostr.write((const char *)&c0_, sizeof(c0_));
    ostr.write((const char *)&c1_, sizeof(c1_));

    if (dense_)
    {
        dense_bit_vector_.save(ostr);
    }
    else
    {
        bit_vector_.Save(ostr);
    }
}

void WaveletTreeNode::load(std::istream &istr)
{
    istr.read((char *)&c0_, sizeof(c0_));
    istr.read((char *)&c1_, sizeof(c1_));

    if (dense_)
    {
        dense_bit_vector_.load(istr);
    }
    else
    {
        bit_vector_.Load(istr);
    }
}

bool WaveletTreeNode::operator>(const WaveletTreeNode &rhs) const
{
    return freq_ > rhs.freq_;
}

}
}

NS_IZENELIB_AM_END
