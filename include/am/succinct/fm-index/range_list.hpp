#ifndef _FM_INDEX_RANGE_LIST_HPP
#define _FM_INDEX_RANGE_LIST_HPP

#include "wavelet_tree_node.hpp"
#include <boost/memory.hpp>

#include <boost/tuple/tuple.hpp>
#include <stdlib.h>
#include <memory>

using boost::stl_allocator;

#define cacheline_size() 64

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{
typedef boost::tuple<size_t, size_t, double> pattern_tuple_type;
typedef std::vector<boost::tuple<size_t, size_t, double>, stl_allocator<boost::tuple<size_t, size_t, double> > > pattern_tuple_list_type;
typedef std::vector<std::pair<size_t, size_t>, stl_allocator<std::pair<size_t, size_t> > > filter_list_type;

namespace detail
{

static double getScore(const pattern_tuple_list_type &patterns)
{
    double score = 0.0;

    for (pattern_tuple_list_type::const_iterator it = patterns.begin();
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
            const pattern_tuple_list_type &patterns)
        : level_(level)
        , sym_(sym)
        , node_(node)
        , patterns_(patterns)
    {
        score_ = detail::getScore(patterns);
    }

    PatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            size_t pattern_count,
            boost::auto_alloc& alloc)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , patterns_(alloc)
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

    bool addPattern(const pattern_tuple_type &pattern)
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
    pattern_tuple_list_type patterns_;
}__attribute__((aligned(cacheline_size())));

class FilteredPatternList
{
public:
    FilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            const filter_list_type &filters,
            const pattern_tuple_list_type &patterns)
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
            size_t filter_count, size_t pattern_count,
            boost::auto_alloc& alloc)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , filters_(alloc)
        , patterns_(alloc)
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

    bool addPattern(const pattern_tuple_type &pattern)
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
    filter_list_type filters_;
    pattern_tuple_list_type patterns_;
}__attribute__((aligned(cacheline_size())));

template <class WaveletTreeType>
class FilterList
{
public:
    FilterList(
            const WaveletTreeType *tree, const WaveletTreeNode *node,
            const std::vector<std::pair<size_t, size_t> > &filters,
            boost::auto_alloc& alloc)
        : tree_(tree) , node_(node)
        , filters_(alloc)
    {
        filters_.resize(filters.size());
        for(unsigned i = 0; i < filters.size(); ++i)
            filters_[i] = filters[i];
    }

    FilterList(
            const WaveletTreeType *tree, const WaveletTreeNode *node,
            size_t filter_count,
            boost::auto_alloc& alloc)
        : tree_(tree) , node_(node), filters_(alloc)
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
    std::vector<std::pair<size_t, size_t>, stl_allocator<std::pair<size_t, size_t> > > filters_;
}__attribute__((aligned(cacheline_size())));

template <class WaveletTreeType>
class AuxFilteredPatternList
{
public:
    typedef std::vector<FilterList<WaveletTreeType> *, stl_allocator<FilterList<WaveletTreeType> * > > auxfilter_type;

    AuxFilteredPatternList(
            size_t level, uint64_t sym,
            const WaveletTreeNode *node,
            const auxfilter_type &aux_filters,
            const pattern_tuple_list_type&patterns,
            boost::auto_alloc& alloc)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , aux_filters_(aux_filters)
        , patterns_(patterns)
        , recyc_aux_filters_(alloc)
        , alloc_(alloc)
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
            size_t aux_filter_count, size_t pattern_count,
            boost::auto_alloc& alloc)
        : level_(level)
        , sym_(sym)
        , score_()
        , node_(node)
        , aux_filters_(alloc)
        , patterns_(alloc)
        , recyc_aux_filters_(alloc)
        , alloc_(alloc)
    {
        aux_filters_.reserve(aux_filter_count);
        patterns_.reserve(pattern_count);

        recyc_aux_filters_.reserve(aux_filter_count);
    }

    ~AuxFilteredPatternList()
    {
        /*
        for (size_t i = 0; i < aux_filters_.size(); ++i)
        {
            delete aux_filters_[i];
        }

        for (size_t i = 0; i < recyc_aux_filters_.size(); ++i)
        {
            delete recyc_aux_filters_[i];
        }*/
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

    FilterList<WaveletTreeType> *getAuxFilter(
            const WaveletTreeType *tree, const WaveletTreeNode *node,
            size_t filter_count)
    {
        if (recyc_aux_filters_.empty())
        {
            //return new FilterList<WaveletTreeType>(tree, node, filter_count);
            return BOOST_NEW(alloc_,FilterList<WaveletTreeType>)(tree, node, filter_count, alloc_);
        }
        else
        {
            FilterList<WaveletTreeType> *filter = recyc_aux_filters_.back();
            filter->reset(tree, node);
            recyc_aux_filters_.pop_back();
            return filter;
        }
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
    auxfilter_type aux_filters_;
    pattern_tuple_list_type patterns_;
    auxfilter_type recyc_aux_filters_;
    boost::auto_alloc& alloc_;
}__attribute__((aligned(cacheline_size())));

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
