#ifndef _FM_INDEX_RANGE_LIST_HPP
#define _FM_INDEX_RANGE_LIST_HPP

#include "wavelet_tree_node.hpp"

#include <boost/tuple/tuple.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

class RangeList
{
public:
    RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node, const std::vector<boost::tuple<size_t, size_t, double> > &ranges);

    RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node, size_t capacity);

    ~RangeList();

    void addRange(const boost::tuple<size_t, size_t, double> &range);

    void calcScore();

    bool operator<(const RangeList &rhs) const;

public:
    size_t level_;
    uint64_t sym_;
    double score_;
    const WaveletTreeNode *node_;
    std::vector<boost::tuple<size_t, size_t, double> > ranges_;
};

class FilteredRangeList
{
public:
    FilteredRangeList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            const std::vector<std::pair<size_t, size_t> > &filters,
            const std::vector<boost::tuple<size_t, size_t, double> > &ranges);

    FilteredRangeList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            size_t filter_count, size_t range_count);

    ~FilteredRangeList();

    void addFilter(const std::pair<size_t, size_t> &filter);

    void addRange(const boost::tuple<size_t, size_t, double> &range);

    void calcScore();

    bool operator<(const FilteredRangeList &rhs) const;

public:
    size_t level_;
    uint64_t sym_;
    double score_;
    const WaveletTreeNode *node_;
    std::vector<std::pair<size_t, size_t> > filters_;
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
    bool operator()(izenelib::am::succinct::fm_index::RangeList * const &p1, izenelib::am::succinct::fm_index::RangeList * const &p2)
    {
        return *p1 < *p2;
    }
};

template <class T>
struct less<pair<izenelib::am::succinct::fm_index::RangeList *, T> >
{
    bool operator()(pair<izenelib::am::succinct::fm_index::RangeList *, T> const &p1, pair<izenelib::am::succinct::fm_index::RangeList *, T> const &p2)
    {
        return *p1.first < *p2.first;
    }
};

template <>
struct less<izenelib::am::succinct::fm_index::FilteredRangeList *>
{
    bool operator()(izenelib::am::succinct::fm_index::FilteredRangeList * const &p1, izenelib::am::succinct::fm_index::FilteredRangeList * const &p2)
    {
        return *p1 < *p2;
    }
};

template <class T>
struct less<pair<izenelib::am::succinct::fm_index::FilteredRangeList *, T> >
{
    bool operator()(pair<izenelib::am::succinct::fm_index::FilteredRangeList *, T> const &p1, pair<izenelib::am::succinct::fm_index::FilteredRangeList *, T> const &p2)
    {
        return *p1.first < *p2.first;
    }
};

}

#endif
