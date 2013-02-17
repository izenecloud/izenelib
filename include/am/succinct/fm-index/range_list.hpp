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

    void reset(size_t level, uint64_t sym, const WaveletTreeNode *node)
    {
        level_ = level;
        sym_ = sym;
        score_ = 0.0;
        node_ = node;
        patterns_.clear();
    }

    bool addPattern(const boost::tuple<size_t, size_t, double> &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
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
        if (!filters_.empty())
        {
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

    void reset(size_t level, uint64_t sym, const WaveletTreeNode *node)
    {
        level_ = level;
        sym_ = sym;
        score_ = 0.0;
        node_ = node;
        filters_.clear();
        patterns_.clear();
    }

    bool addFilter(const std::pair<size_t, size_t> &filter)
    {
        if (filter.first < filter.second)
        {
            filters_.push_back(filter);
            return true;
        }
        return false;
    }

    bool addPattern(const boost::tuple<size_t, size_t, double> &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
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
    }

    FilterList(
            const WaveletTreeType *tree, const WaveletTreeNode *node,
            size_t filter_count)
        : tree_(tree) , node_(node)
    {
        filters_.reserve(filter_count);
    }

    ~FilterList() {}

    void reset(const WaveletTreeType *tree, const WaveletTreeNode *node)
    {
        tree_ = tree;
        node_ = node;
        filters_.clear();
    }

    bool addFilter(const std::pair<size_t, size_t> &filter)
    {
        if (filter.first < filter.second)
        {
            filters_.push_back(filter);
            return true;
        }
        return false;
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
            const std::vector<boost::tuple<size_t, size_t, double> > &patterns)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , aux_filters_(aux_filters)
        , patterns_(patterns)
    {
        if (!aux_filters_.empty())
        {
            score_ = detail::getScore(patterns_);
            recyc_aux_filters_.reserve(aux_filters_.size());
        }
    }

    AuxFilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            size_t aux_filter_count, size_t pattern_count, size_t filter_count)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
    {
        aux_filters_.reserve(aux_filter_count);
        patterns_.reserve(pattern_count);

        recyc_aux_filters_.resize(aux_filter_count);
        for (size_t i = 0; i < recyc_aux_filters_.size(); ++i)
        {
            recyc_aux_filters_[i] = new FilterList<WaveletTreeType>(NULL, NULL, filter_count);
        }
    }

    ~AuxFilteredPatternList()
    {
        for (size_t i = 0; i < aux_filters_.size(); ++i)
        {
            delete aux_filters_[i];
        }

        for (size_t i = 0; i < recyc_aux_filters_.size(); ++i)
        {
            delete recyc_aux_filters_[i];
        }
    }

    void reset(size_t level, uint64_t sym, const WaveletTreeNode *node)
    {
        level_ = level;
        sym_ = sym;
        score_ = 0.0;
        node_ = node;
        recyc_aux_filters_.insert(recyc_aux_filters_.end(), aux_filters_.begin(), aux_filters_.end());
        aux_filters_.clear();
        patterns_.clear();
    }

    bool addAuxFilter(FilterList<WaveletTreeType> *aux_filter)
    {
        if (!aux_filter)
        {
            return false;
        }
        else if (aux_filter->filters_.empty())
        {
            recyc_aux_filters_.push_back(aux_filter);
            return false;
        }
        else
        {
            aux_filters_.push_back(aux_filter);
            return true;
        }
    }

    bool addPattern(const boost::tuple<size_t, size_t, double> &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
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
    std::vector<boost::tuple<size_t, size_t, double> > patterns_;
    std::vector<FilterList<WaveletTreeType> *> recyc_aux_filters_;
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
