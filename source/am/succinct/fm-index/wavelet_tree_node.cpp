#include <am/succinct/fm-index/wavelet_tree_node.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

WaveletTreeNode::WaveletTreeNode()
    : left_(), right_(), parent_()
    , freq_(), c0_(), c1_()
    , len_(), ones_()
{
}

WaveletTreeNode::WaveletTreeNode(uint64_t c, size_t freq)
    : left_(), right_(), parent_()
    , freq_(freq), c0_(c), c1_()
    , len_(), ones_()
{
}

WaveletTreeNode::~WaveletTreeNode()
{
}

WaveletTreeNode::WaveletTreeNode(WaveletTreeNode *left, WaveletTreeNode *right)
    : left_(left), right_(right), parent_()
    , freq_(left->freq_ + right->freq_), c0_(), c1_()
    , len_(), ones_()
{
    left->parent_ = this;
    right->parent_ = this;
}

void WaveletTreeNode::resize(size_t len)
{
    len_ = len;
    size_t index = len_ / 32;
    if (index >= raw_array_.size())
        raw_array_.resize(index + 1);
}

void WaveletTreeNode::setBit(size_t pos)
{
    size_t index = pos / 32, offset = pos % 32;
    if (pos >= len_)
    {
        len_ = pos + 1;
        if (index >= raw_array_.size())
            raw_array_.resize(index + 1);
    }
    raw_array_[index] |= 1U << offset;
    ++ones_;
}

void WaveletTreeNode::unsetBit(size_t pos)
{
    size_t index = pos / 32/*, offset = pos % 32*/;
    if (pos >= len_)
    {
        len_ = pos + 1;
        if (index >= raw_array_.size())
            raw_array_.resize(index + 1);
    }
//  raw_array_[index] &= ~(1U << offset);
}

void WaveletTreeNode::changeBit(bool bit, size_t pos)
{
    if (bit) setBit(pos);
    else unsetBit(pos);
}

void WaveletTreeNode::append0()
{
    size_t index = len_ / 32/*, offset = len_ % 32*/;
    if (index >= raw_array_.size())
        raw_array_.resize(index + 1);
//  raw_array_[index] &= ~(1U << offset);
    ++len_;
}

void WaveletTreeNode::append1()
{
    size_t index = len_ / 32, offset = len_ % 32;
    if (index >= raw_array_.size())
        raw_array_.resize(index + 1);
    raw_array_[index] |= 1U << offset;
    ++len_;
    ++ones_;
}

void WaveletTreeNode::appendBit(bool bit)
{
    if (bit) append1();
    else append0();
}

void WaveletTreeNode::build()
{
    if (!raw_array_.empty())
    {
        bit_vector_.build(raw_array_, len_);
        std::vector<uint32_t>().swap(raw_array_);
    }
}

size_t WaveletTreeNode::length() const
{
    return len_;
}

size_t WaveletTreeNode::getSize() const
{
    return sizeof(WaveletTreeNode)
        + bit_vector_.getSize() - sizeof(RRRBitVector)
        + sizeof(raw_array_[0]) * raw_array_.size();
}

void WaveletTreeNode::save(std::ostream &ostr) const
{
    ostr.write((const char *)&freq_, sizeof(freq_));
    ostr.write((const char *)&c0_,   sizeof(c0_));
    ostr.write((const char *)&c1_,   sizeof(c1_));
    ostr.write((const char *)&len_,  sizeof(len_));
    ostr.write((const char *)&ones_, sizeof(ones_));

    bit_vector_.save(ostr);
}

void WaveletTreeNode::load(std::istream &istr)
{
    istr.read((char *)&freq_, sizeof(freq_));
    istr.read((char *)&c0_,   sizeof(c0_));
    istr.read((char *)&c1_,   sizeof(c1_));
    istr.read((char *)&len_,  sizeof(len_));
    istr.read((char *)&ones_, sizeof(ones_));

    bit_vector_.load(istr);
}

bool WaveletTreeNode::operator>(const WaveletTreeNode &rhs) const
{
    return freq_ > rhs.freq_;
}

}
}

NS_IZENELIB_AM_END
