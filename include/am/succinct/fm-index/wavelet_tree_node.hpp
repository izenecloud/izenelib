#ifndef _FM_INDEX_WAVELET_TREE_NODE_HPP
#define _FM_INDEX_WAVELET_TREE_NODE_HPP

#include <am/succinct/rsdic/RSDic.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

class WaveletTreeNode
{
public:
    WaveletTreeNode();
    WaveletTreeNode(uint64_t c, size_t freq);
    WaveletTreeNode(WaveletTreeNode *left, WaveletTreeNode *right);

    ~WaveletTreeNode();

    void resize(size_t len);

    void setBit(size_t pos);
    void unsetBit(size_t pos);
    void changeBit(bool bit, size_t pos);

    void append0();
    void append1();
    void appendBit(bool bit);

    void build();

    size_t length() const;
    size_t getSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

    bool operator>(const WaveletTreeNode &rhs) const;

public:
    WaveletTreeNode *left_;
    WaveletTreeNode *right_;
    WaveletTreeNode *parent_;

    size_t freq_;
    uint64_t c0_;
    uint64_t c1_;

    size_t len_;
    std::vector<uint64_t> raw_array_;
    rsdic::RSDic bit_vector_;
};

}
}

NS_IZENELIB_AM_END

namespace std
{

template <>
struct greater<izenelib::am::succinct::fm_index::WaveletTreeNode *>
{
    bool operator()(izenelib::am::succinct::fm_index::WaveletTreeNode *p1, izenelib::am::succinct::fm_index::WaveletTreeNode *p2)
    {
        if (!p1) return false;
        if (!p2) return true;
        return *p1 > *p2;
    }
};

}

#endif
