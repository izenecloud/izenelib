#ifndef _FM_INDEX_RANGE_LIST_HPP
#define _FM_INDEX_RANGE_LIST_HPP

#include "wavelet_tree_node.hpp"

#include <boost/tuple/tuple.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

namespace detail
{

static double getScore(const std::vector<boost::tuple<size_t, size_t, double> > &patterns)
{
    double score = 0.0;

    for (std::vector<boost::tuple<size_t, size_t, double> >::const_iterator it = patterns.begin();
            it != patterns.end(); ++it)
    {
        score += it->get<2>();
    }

    return score;
}

static struct InvalidRange
{
    bool operator()(const std::pair<size_t, size_t> &range) const
    {
        return range.first >= range.second;
    }

    bool operator()(const boost::tuple<size_t, size_t, double> &range) const
    {
        return range.get<0>() >= range.get<1>();
    }
} InvalidRange;

}

class PatternList
{
public:
    PatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            const std::vector<boost::tuple<size_t, size_t, double> > &patterns)
        : level_(level)
        , sym_(sym)
        , node_(node)
        , patterns_(patterns)
    {
//      patterns_.erase(std::remove_if(patterns_.begin(), patterns_.end(), detail::InvalidRange), patterns_.end());
        score_ = detail::getScore(patterns_);
    }

    PatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            size_t pattern_count)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
    {
        patterns_.reserve(pattern_count);
    }

    ~PatternList() {}

    void addPattern(const boost::tuple<size_t, size_t, double> &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
            patterns_.push_back(pattern);
    }

    void calcScore()
    {
        score_ = detail::getScore(patterns_);
    }

    bool operator<(const PatternList &rhs) const
    {
        return score_ < rhs.score_ || (score_ == rhs.score_ && level_ < rhs.level_);
    }

public:
    size_t level_;
    uint64_t sym_;
    double score_;
    const WaveletTreeNode *node_;
    std::vector<boost::tuple<size_t, size_t, double> > patterns_;
};

class FilteredPatternList
{
public:
    FilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            const std::vector<std::pair<size_t, size_t> > &filters,
            const std::vector<boost::tuple<size_t, size_t, double> > &patterns)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , filters_(filters)
        , patterns_(patterns)
    {
//      filters_.erase(std::remove_if(filters_.begin(), filters_.end(), detail::InvalidRange), filters_.end());
        if (!filters_.empty())
        {
//          patterns_.erase(std::remove_if(patterns_.begin(), patterns_.end(), detail::InvalidRange), patterns_.end());
            score_ = detail::getScore(patterns_);
        }
    }

    FilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            size_t filter_count, size_t pattern_count)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
    {
        filters_.reserve(filter_count);
        patterns_.reserve(pattern_count);
    }

    ~FilteredPatternList() {}

    void addFilter(const std::pair<size_t, size_t> &filter)
    {
        if (filter.first < filter.second)
            filters_.push_back(filter);
    }

    void addPattern(const boost::tuple<size_t, size_t, double> &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
            patterns_.push_back(pattern);
    }

    void calcScore()
    {
        score_ = detail::getScore(patterns_);
    }

    bool operator<(const FilteredPatternList &rhs) const
    {
        return score_ < rhs.score_ || (score_ == rhs.score_ && level_ < rhs.level_);
    }

public:
    size_t level_;
    uint64_t sym_;
    double score_;
    const WaveletTreeNode *node_;
    std::vector<std::pair<size_t, size_t> > filters_;
    std::vector<boost::tuple<size_t, size_t, double> > patterns_;
};

template <class WaveletTreeType>
class FilterList
{
public:
    FilterList(
            const WaveletTreeType *tree, const WaveletTreeNode *node,
            const std::vector<std::pair<size_t, size_t> > &filters)
        : tree_(tree) , node_(node)
        , filters_(filters)
    {
//      filters_.erase(std::remove_if(filters_.begin(), filters_.end(), detail::InvalidRange), filters_.end());
    }

    FilterList(
            const WaveletTreeType *tree, const WaveletTreeNode *node,
            size_t filter_count)
        : tree_(tree) , node_(node)
    {
        filters_.reserve(filter_count);
    }

    ~FilterList() {}

    void addFilter(const std::pair<size_t, size_t> &filter)
    {
        if (filter.first < filter.second)
            filters_.push_back(filter);
    }

public:
    const WaveletTreeType *tree_;
    const WaveletTreeNode *node_;
    std::vector<std::pair<size_t, size_t> > filters_;
};

template <class WaveletTreeType>
class AuxFilteredPatternList
{
public:
    AuxFilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            const std::vector<FilterList<WaveletTreeType> *> &aux_filters,
            const std::vector<std::pair<size_t, size_t> > &filters,
            const std::vector<boost::tuple<size_t, size_t, double> > &patterns)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , aux_filters_(aux_filters)
        , filters_(filters)
        , patterns_(patterns)
    {
//      filters_.erase(std::remove_if(filters_.begin(), filters_.end(), detail::InvalidRange), filters_.end());
        if (!aux_filters_.empty() || !filters_.empty())
        {
//          patterns_.erase(std::remove_if(patterns_.begin(), patterns_.end(), detail::InvalidRange), patterns_.end());
            score_ = detail::getScore(patterns_);
        }
    }

    AuxFilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            size_t aux_filter_count, size_t filter_count, size_t pattern_count)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
    {
        aux_filters_.reserve(aux_filter_count);
        filters_.reserve(filter_count);
        patterns_.reserve(pattern_count);
    }

    ~AuxFilteredPatternList()
    {
        for (size_t i = 0; i < aux_filters_.size(); ++i)
        {
            delete aux_filters_[i];
        }
    }

    void addFilter(const std::pair<size_t, size_t> &filter)
    {
        if (filter.first < filter.second)
            filters_.push_back(filter);
    }

    void addAuxFilter(FilterList<WaveletTreeType> *aux_filter)
    {
        if (aux_filter)
        {
            if (!aux_filter->filters_.empty())
                aux_filters_.push_back(aux_filter);
            else
                delete aux_filter;
        }
    }

    void addPattern(const boost::tuple<size_t, size_t, double> &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
            patterns_.push_back(pattern);
    }

    void calcScore()
    {
        score_ = detail::getScore(patterns_);
    }

    bool operator<(const AuxFilteredPatternList &rhs) const
    {
        return score_ < rhs.score_ || (score_ == rhs.score_ && level_ < rhs.level_);
    }

public:
    size_t level_;
    uint64_t sym_;
    double score_;
    const WaveletTreeNode *node_;
    std::vector<FilterList<WaveletTreeType> *> aux_filters_;
    std::vector<std::pair<size_t, size_t> > filters_;
    std::vector<boost::tuple<size_t, size_t, double> > patterns_;
};

}
}

NS_IZENELIB_AM_END

namespace std
{

template <>
struct less<izenelib::am::succinct::fm_index::PatternList *>
{
    bool operator()(izenelib::am::succinct::fm_index::PatternList * const &p1, izenelib::am::succinct::fm_index::PatternList * const &p2)
    {
        return *p1 < *p2;
    }
};

template <class T>
struct less<pair<izenelib::am::succinct::fm_index::PatternList *, T> >
{
    bool operator()(pair<izenelib::am::succinct::fm_index::PatternList *, T> const &p1, pair<izenelib::am::succinct::fm_index::PatternList *, T> const &p2)
    {
        return *p1.first < *p2.first;
    }
};

template <>
struct less<izenelib::am::succinct::fm_index::FilteredPatternList *>
{
    bool operator()(izenelib::am::succinct::fm_index::FilteredPatternList * const &p1, izenelib::am::succinct::fm_index::FilteredPatternList * const &p2)
    {
        return *p1 < *p2;
    }
};

template <class T>
struct less<pair<izenelib::am::succinct::fm_index::FilteredPatternList *, T> >
{
    bool operator()(pair<izenelib::am::succinct::fm_index::FilteredPatternList *, T> const &p1, pair<izenelib::am::succinct::fm_index::FilteredPatternList *, T> const &p2)
    {
        return *p1.first < *p2.first;
    }
};

template <class W>
struct less<izenelib::am::succinct::fm_index::AuxFilteredPatternList<W> *>
{
    bool operator()(izenelib::am::succinct::fm_index::AuxFilteredPatternList<W> * const &p1, izenelib::am::succinct::fm_index::AuxFilteredPatternList<W> * const &p2)
    {
        return *p1 < *p2;
    }
};

template <class W, class T>
struct less<pair<izenelib::am::succinct::fm_index::AuxFilteredPatternList<W> *, T> >
{
    bool operator()(pair<izenelib::am::succinct::fm_index::AuxFilteredPatternList<W> *, T> const &p1, pair<izenelib::am::succinct::fm_index::AuxFilteredPatternList<W> *, T> const &p2)
    {
        return *p1.first < *p2.first;
    }
};

}

#endif
