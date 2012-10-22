#ifndef _FM_INDEX_WAVELET_TREE_HPP
#define _FM_INDEX_WAVELET_TREE_HPP

#include "const.hpp"
#include "wavelet_tree_node.hpp"
#include <3rdparty/boost/container/priority_deque.hpp>

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
            const std::vector<boost::tuple<size_t, size_t, double> > &ranges,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results) const = 0;

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
    RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node, const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
        : level_(level)
        , sym_(sym)
        , score_(getScore_(level, ranges))
        , node_(node)
        , ranges_(ranges)
    {
    };

    RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
    {
    };

    ~RangeList() {}

    void addRange(const boost::tuple<size_t, size_t, double> &range)
    {
        ranges_.push_back(range);
    };

    void calcScore()
    {
        score_ = getScore_(level_, ranges_);
    }

    bool operator<(const RangeList &rhs) const
    {
        return score_ < rhs.score_;
    };

private:
    double getScore_(size_t level, const std::vector<boost::tuple<size_t, size_t, double> > &ranges) const
    {
        double score = 0.0;

        for (std::vector<boost::tuple<size_t, size_t, double> >::const_iterator it = ranges.begin();
                it != ranges.end(); ++it)
        {
            if (it->get<1>() > it->get<0>())
                score += it->get<2>();
        }

        return score * (level + 1);
    }

public:
    size_t level_;
    uint64_t sym_;
    double score_;
    const WaveletTreeNode *node_;
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
