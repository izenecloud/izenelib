#ifndef _FM_INDEX_WAVELET_TREE_NODE_HPP
#define _FM_INDEX_WAVELET_TREE_NODE_HPP

#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/dbitv/dbitv.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

class WaveletTreeNode
{
public:
    WaveletTreeNode(bool support_select, bool dense);
    WaveletTreeNode(size_t c, size_t freq, bool support_select, bool dense);
    WaveletTreeNode(WaveletTreeNode *left, WaveletTreeNode *right, bool support_select, bool dense);

    ~WaveletTreeNode();

    void resize(size_t len);

    void setBit(size_t pos);
    void unsetBit(size_t pos);
    void changeBit(bool bit, size_t pos);

    void append0();
    void append1();
    void append(bool bit);

    void build();

    bool access(size_t pos) const;
    bool access(size_t pos, size_t& rank) const;

    size_t rank0(size_t pos) const;
    size_t rank1(size_t pos) const;
    size_t rank(size_t pos, bool bit) const;

    size_t select0(size_t ind) const;
    size_t select1(size_t ind) const;
    size_t select(size_t ind, bool bit) const;

    size_t length() const;
    size_t allocSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

    bool operator>(const WaveletTreeNode &rhs) const;

public:
    WaveletTreeNode *left_;
    WaveletTreeNode *right_;
    WaveletTreeNode *parent_;

    size_t c0_;
    size_t c1_;

    rsdic::RSDic bit_vector_;
    dense::DBitV dense_bit_vector_;

private:
    bool dense_;
    size_t freq_;
    size_t len_;
    std::vector<size_t> raw_array_;
};

}
}

NS_IZENELIB_AM_END

namespace std
{

template <>
struct greater<izenelib::am::succinct::fm_index::WaveletTreeNode *>
{
    bool operator()(izenelib::am::succinct::fm_index::WaveletTreeNode * const &p1, izenelib::am::succinct::fm_index::WaveletTreeNode * const &p2)
    {
        return *p1 > *p2;
    }
};

}

#endif
