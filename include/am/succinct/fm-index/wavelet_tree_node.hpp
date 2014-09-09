#ifndef _FM_INDEX_WAVELET_TREE_NODE_HPP
#define _FM_INDEX_WAVELET_TREE_NODE_HPP

#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/dbitv/dbitv.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class T>
class WaveletTreeNode
{
public:
    typedef T bitmap_type;
    typedef WaveletTreeNode<T> self_type;

    WaveletTreeNode(bool support_select)
        : left_(), right_(), parent_()
        , c0_(), c1_()
        , freq_(), len_()
        , bitmap_(support_select)
    {
    }

    WaveletTreeNode(uint64_t c, size_t freq, bool support_select)
        : left_(), right_(), parent_()
        , c0_(c), c1_()
        , freq_(freq), len_()
        , bitmap_(support_select)
    {
    }

    WaveletTreeNode(self_type *left, self_type *right, bool support_select)
        : left_(left), right_(right), parent_()
        , c0_(), c1_()
        , freq_(left->freq_ + right->freq_), len_()
        , raw_array_((freq_ + kBlockSize - 1) / kBlockSize)
        , bitmap_(support_select)
    {
        left->parent_ = this;
        right->parent_ = this;
    }

    ~WaveletTreeNode() {}

    void resize(size_t len)
    {
        len_ = len;
        raw_array_.resize((len_ + kBlockSize - 1) / kBlockSize);
    }

    void setBit(size_t pos)
    {
        assert(pos < len_);
        //if (pos >= len_)
        //{
        //    len_ = pos + 1;
        //    raw_array_.resize(pos / kBlockSize + 1);
        //}
        raw_array_[pos / kBlockSize] |= 1LLU << pos;
    }

    void unsetBit(size_t pos)
    {
        assert(pos < len_);
        //if (pos >= len_)
        //{
        //    len_ = pos + 1;
        //    raw_array_.resize(pos / kBlockSize + 1);
        //}
        //raw_array_[pos / kBlockSize] &= ~(1LLU << pos);
    }

    void changeBit(bool bit, size_t pos)
    {
        if (bit) setBit(pos);
        else unsetBit(pos);
    }

    void append0()
    {
        assert(len_ / kBlockSize < raw_array_.size());
        //raw_array_.resize(len_ / kBlockSize + 1);
        //raw_array_[len_ / kBlockSize] &= ~(1LLU << len_);
        ++len_;
    }

    void append1()
    {
        assert(len_ / kBlockSize < raw_array_.size());
        //raw_array_.resize(len_ / kBlockSize + 1);
        raw_array_[len_ / kBlockSize] |= 1LLU << len_;
        ++len_;
    }

    void append(bool bit)
    {
        if (bit) append1();
        else append0();
    }

    void build()
    {
        if (!raw_array_.empty())
        {
            bitmap_.build(raw_array_, len_);
            std::vector<uint64_t>().swap(raw_array_);
        }
    }

    inline bool access(size_t pos) const { return bitmap_.access(pos); }
    inline bool access(size_t pos, size_t& rank) const { return bitmap_.access(pos, rank); }

    inline size_t rank0(size_t pos) const { return bitmap_.rank0(pos); }
    inline size_t rank1(size_t pos) const { return bitmap_.rank1(pos); }
    inline size_t rank(size_t pos, bool bit) const { return bitmap_.rank(pos, bit); }

    inline size_t select0(size_t ind) const { return bitmap_.select0(ind); }
    inline size_t select1(size_t ind) const { return bitmap_.select1(ind); }
    inline size_t select(size_t ind, bool bit) const { return bitmap_.select(ind, bit); }

    inline size_t length() const { return bitmap_.length(); }

    size_t allocSize() const
    {
        return sizeof(self_type)
            + bitmap_.allocSize() - sizeof(bitmap_type)
            + sizeof(raw_array_[0]) * raw_array_.size();
    }

    void save(std::ostream &ostr) const
    {
        ostr.write((const char *)&c0_, sizeof(c0_));
        ostr.write((const char *)&c1_, sizeof(c1_));

        bitmap_.save(ostr);
    }

    void load(std::istream &istr)
    {
        istr.read((char *)&c0_, sizeof(c0_));
        istr.read((char *)&c1_, sizeof(c1_));

        bitmap_.load(istr);
    }

    bool operator<(const self_type &rhs) const
    {
        return freq_ < rhs.freq_;
    }

public:
    self_type *left_;
    self_type *right_;
    self_type *parent_;

    uint64_t c0_;
    uint64_t c1_;

private:
    size_t freq_;
    size_t len_;

    std::vector<uint64_t> raw_array_;
    bitmap_type bitmap_;
};

}
}

NS_IZENELIB_AM_END

namespace std
{

template <class T>
struct less<izenelib::am::succinct::fm_index::WaveletTreeNode<T> *>
{
    bool operator()(izenelib::am::succinct::fm_index::WaveletTreeNode<T> * const &p1, izenelib::am::succinct::fm_index::WaveletTreeNode<T> * const &p2)
    {
        return *p1 < *p2;
    }
};

}

#endif
