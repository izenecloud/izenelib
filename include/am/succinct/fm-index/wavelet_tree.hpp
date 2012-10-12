#ifndef _FM_INDEX_WAVELET_TREE_HPP
#define _FM_INDEX_WAVELET_TREE_HPP

#include "wavelet_tree_node.hpp"

#include <boost/tuple/tuple.hpp>


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

    virtual void intersect(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t thres,
            std::vector<char_type> &result) const = 0;

    virtual void topKUnion(
            const std::vector<std::pair<size_t, size_t> > &ranges,
            size_t topK,
            std::vector<char_type> &result) const = 0;

    virtual size_t getOcc(char_type c) const = 0;

    virtual size_t length() const = 0;
    virtual size_t allocSize() const = 0;

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

class RangeList
{
public:
    RangeList(size_t start, size_t end, uint64_t sym, const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
        : sym_(sym)
        , start_(start)
        , end_(end)
        , score_(getScore_(ranges))
        , ranges_(ranges)
    {
    };

    RangeList(size_t start, size_t end, uint64_t sym)
        : sym_(sym)
        , start_(start)
        , end_(end)
        , score_()
    {
    };

    ~RangeList() {}

    void addRange(size_t sp, size_t ep, double score)
    {
        ranges_.push_back(boost::make_tuple(sp, ep, score));
        score_ += getScore_(ranges_.back());
    };

    bool operator<(const RangeList &rhs) const
    {
        return score_ < rhs.score_;
    };

private:
    double getScore_(const boost::tuple<size_t, size_t, double> &range) const
    {
        switch (range.get<0>() - range.get<1>())
        {
        case 0: return 0.0;
        case 1: return range.get<2>();
        case 2: return range.get<2>() * 1.3;
        default : return range.get<2>() * 1.5;
        }
    }

    double getScore_(const std::vector<boost::tuple<size_t, size_t, double> > &ranges) const
    {
        double score = 0.0;

        for (size_t i = 0; i < ranges.size(); ++i)
        {
            score += getScore_(ranges[i]);
        }

        return score;
    }

public:
    uint64_t sym_;
    size_t start_;
    size_t end_;
    size_t score_;
    std::vector<boost::tuple<size_t, size_t, double> > ranges_;
};

}
}

NS_IZENELIB_AM_END

namespace std
{

template <>
struct less<izenelib::am::succinct::fm_index::RangeList *>
{
    bool operator()(izenelib::am::succinct::fm_index::RangeList *p1, izenelib::am::succinct::fm_index::RangeList *p2)
    {
        return *p1 < *p2;
    }
};

}

#endif
