#ifndef _FM_INDEX_WAVELET_TREE_HPP
#define _FM_INDEX_WAVELET_TREE_HPP

#include <types.h>

#include <vector>
#include <iostream>


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

    WaveletTree(size_t alphabet_num) : alphabet_num_(alphabet_num) {}
    virtual ~WaveletTree() {}

    virtual void build(const char_type *char_seq, size_t len) = 0;

    virtual char_type access(size_t pos) const = 0;
    virtual char_type access(size_t pos, size_t &rank) const = 0;

    virtual size_t rank(char_type c, size_t pos) const = 0;
    virtual size_t select(char_type c, size_t rank) const = 0;

    virtual size_t getOcc(char_type c) const = 0;

    virtual size_t length() const = 0;
    virtual size_t getSize() const = 0;

    inline size_t getAlphabetNum() const
    {
        return alphabet_num_;
    }

    virtual void save(std::ostream &ostr) const
    {
        ostr.write((const char *)&alphabet_num_, sizeof(alphabet_num_));
    }

    virtual void load(std::istream &istr)
    {
        istr.read((char *)&alphabet_num_, sizeof(alphabet_num_));
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
};

}
}

NS_IZENELIB_AM_END

#endif
