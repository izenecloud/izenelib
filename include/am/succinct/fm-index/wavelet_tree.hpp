#ifndef _FM_INDEX_WAVELET_TREE_HPP
#define _FM_INDEX_WAVELET_TREE_HPP

#include "const.hpp"
#include <am/succinct/utils.hpp>


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

    WaveletTree(uint64_t alphabet_num, bool support_select)
        : alphabet_num_(alphabet_num)
        , alphabet_bit_num_(SuccinctUtils::log2(alphabet_num_ - 1))
        , support_select_(support_select)
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
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &result) const = 0;

//  virtual void topKUnion(
//          const range_list_type &patterns,
//          size_t thres,
//          size_t topK,
//          std::vector<std::pair<double, char_type> > &results,
//          boost::auto_alloc& alloc) const = 0;

//  virtual void topKUnionWithFilters(
//          const range_list_type &filter,
//          const range_list_type &patterns,
//          size_t thres,
//          size_t topK,
//          std::vector<std::pair<double, char_type> > &results,
//          boost::auto_alloc& alloc) const = 0;

    virtual size_t beginOcc(char_type c) const = 0;
    virtual size_t endOcc(char_type c) const = 0;

    virtual size_t length() const = 0;
    virtual size_t allocSize() const = 0;

    inline uint64_t getAlphabetNum() const
    {
        return alphabet_num_;
    }

    inline size_t getAlphabetBitNum() const
    {
        return alphabet_bit_num_;
    }

    inline bool supportSelect() const
    {
        return support_select_;
    }

    virtual void save(std::ostream &ostr) const
    {
        ostr.write((const char *)&alphabet_num_, sizeof(alphabet_num_));
        ostr.write((const char *)&support_select_, sizeof(support_select_));
    }

    virtual void load(std::istream &istr)
    {
        istr.read((char *)&alphabet_num_, sizeof(alphabet_num_));
        istr.read((char *)&support_select_, sizeof(support_select_));
        alphabet_bit_num_ = SuccinctUtils::log2(alphabet_num_ - 1);
    }

    static uint64_t getAlphabetNum(const char_type *char_seq, size_t len)
    {
        uint64_t num = 0;
        for (size_t i = 0; i < len; ++i)
        {
            num = std::max((uint64_t)char_seq[i] + 1, num);
        }

        return num;
    }

protected:
    uint64_t alphabet_num_;
    size_t alphabet_bit_num_;
    bool support_select_;
};

}
}

NS_IZENELIB_AM_END

#endif
