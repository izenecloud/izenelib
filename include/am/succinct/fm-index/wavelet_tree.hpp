#ifndef _FM_INDEX_WAVELET_TREE_HPP
#define _FM_INDEX_WAVELET_TREE_HPP

#include "const.hpp"
#include "range_list.hpp"
#include <3rdparty/boost/priority_deque.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class WaveletTree
{
public:
    typedef CharT char_type;

    WaveletTree(size_t alphabet_num)
        : alphabet_num_(alphabet_num)
        , alphabet_bit_num_()
    {
    }

    virtual ~WaveletTree()
    {
    }

    virtual void build(const char_type *char_seq, size_t len) = 0;

    virtual char_type access(size_t pos) const = 0;
    virtual char_type access(size_t pos, size_t &rank) const = 0;

    virtual size_t rank(char_type c, size_t pos) const = 0;
    virtual size_t select(char_type c, size_t rank) const = 0;

    virtual void intersect(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &result) const = 0;

    virtual void topKUnion(
            const std::vector<boost::tuple<size_t, size_t, double> > &ranges,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results) const = 0;

    virtual void topKUnionWithFilters(
            const std::vector<std::pair<size_t, size_t> > &filter,
            const std::vector<boost::tuple<size_t, size_t, double> > &ranges,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results) const = 0;

    virtual size_t getOcc(char_type c) const = 0;
    virtual WaveletTreeNode *getRoot() const = 0;

    virtual size_t length() const = 0;
    virtual size_t allocSize() const = 0;

    inline size_t getAlphabetNum() const
    {
        return alphabet_num_;
    }

    virtual void save(std::ostream &ostr) const
    {
        ostr.write((const char *)&alphabet_num_,     sizeof(alphabet_num_));
        ostr.write((const char *)&alphabet_bit_num_, sizeof(alphabet_bit_num_));
    }

    virtual void load(std::istream &istr)
    {
        istr.read((char *)&alphabet_num_, sizeof(alphabet_num_));
        istr.read((char *)&alphabet_bit_num_, sizeof(alphabet_bit_num_));
    }

    static size_t getAlphabetNum(const char_type *char_seq, size_t len)
    {
        size_t num = 0;
        for (size_t i = 0; i < len; ++i)
        {
            num = std::max((size_t)char_seq[i] + 1, num);
        }

        return num;
    }

protected:
    size_t alphabet_num_;
    size_t alphabet_bit_num_;
};

}
}

NS_IZENELIB_AM_END

#endif
