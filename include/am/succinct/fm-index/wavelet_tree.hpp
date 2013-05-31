#ifndef _FM_INDEX_WAVELET_TREE_HPP
#define _FM_INDEX_WAVELET_TREE_HPP

#include "const.hpp"
#include "range_list.hpp"


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

    WaveletTree(const std::pair<uint64_t, uint64_t> &symbol_range, bool support_select, bool dense)
        : symbol_range_(symbol_range)
        , level_count_()
        , support_select_(support_select)
        , dense_(dense)
    {
    }

    WaveletTree(bool support_select, bool dense)
        : level_count_()
        , support_select_(support_select)
        , dense_(dense)
    {
    }

    virtual ~WaveletTree()
    {
    }

    const std::pair<uint64_t, uint64_t> &symbolRange() const
    {
        return symbol_range_;
    }

    size_t levelCount() const
    {
        return level_count_;
    }

    virtual void build(char_type *char_seq, size_t len) = 0;

    virtual char_type access(size_t pos) const = 0;
    virtual char_type access(size_t pos, size_t &rank) const = 0;

    virtual size_t rank(char_type c, size_t pos) const = 0;
    virtual size_t select(char_type c, size_t rank) const = 0;

    virtual void intersect(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &result) const = 0;

    virtual void topKUnion(
            const range_list_type &patterns,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const = 0;

    virtual void topKUnionWithFilters(
            const range_list_type &filter,
            const range_list_type &patterns,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const = 0;

    virtual size_t getOcc(char_type c) const = 0;
    virtual WaveletTreeNode *getRoot() const = 0;

    virtual size_t length() const = 0;
    virtual size_t allocSize() const = 0;

    virtual void save(std::ostream &ostr) const
    {
        ostr.write((const char *)&symbol_range_, sizeof(symbol_range_));
        ostr.write((const char *)&support_select_, sizeof(support_select_));
        ostr.write((const char *)&dense_, sizeof(dense_));
    }

    virtual void load(std::istream &istr)
    {
        istr.read((char *)&symbol_range_, sizeof(symbol_range_));
        istr.read((char *)&support_select_, sizeof(support_select_));
        istr.read((char *)&dense_, sizeof(dense_));
    }

    static std::pair<uint64_t, uint64_t> getSymbolRange(const char_type *char_seq, size_t len)
    {
        uint64_t sym_start = 0, sym_end = 0;
        for (size_t i = 0; i < len; ++i)
        {
            sym_start = std::min((uint64_t)char_seq[i], sym_start);
            sym_end = std::max((uint64_t)char_seq[i] + 1, sym_end);
        }

        return std::make_pair(sym_start, sym_end);
    }

protected:
    std::pair<uint64_t, uint64_t> symbol_range_;
    size_t level_count_;
    bool support_select_;
    bool dense_;
};

}
}

NS_IZENELIB_AM_END

#endif
