#ifndef _FM_INDEX_RANGE_LIST_HPP
#define _FM_INDEX_RANGE_LIST_HPP

#include "wavelet_tree_node.hpp"
#include <util/mem_utils.h>

#include <boost/memory.hpp>
#include <boost/tuple/tuple.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

typedef boost::tuple<size_t, size_t, double> range_type;
typedef std::vector<range_type, boost::stl_allocator<range_type> > range_list_type;
typedef std::vector<size_t, boost::stl_allocator<size_t> > head_list_type;

namespace detail
{

static double getPatternScore(const range_list_type &patterns)
{
    double score = 0.0;

    for (range_list_type::const_iterator it = patterns.begin();
            it != patterns.end(); ++it)
    {
        score += it->get<2>();
    }

    return score;
}

static double getSynonymPatternScore(const range_list_type &patterns, const head_list_type &synonyms)
{
    double score = 0.0;

    for (size_t i = 0; i < synonyms.size() - 1; ++i)
    {
        score += patterns[synonyms[i]].get<2>();
    }

    return score;
}

static bool range_compare(const range_type &p1, const range_type &p2)
{
    return p1.get<2>() > p2.get<2>();
}

}

class PatternList
{
public:
    PatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            const range_list_type &patterns)
        : sym_(sym)
        , node_(node)
        , patterns_(patterns)
    {
    }

    PatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            size_t pattern_count,
            boost::auto_alloc& alloc)
        : sym_(sym)
        , node_(node)
        , patterns_(alloc)
    {
        patterns_.reserve(pattern_count);
    }

    ~PatternList() {}

    void reset(uint64_t sym, const WaveletTreeNode<dense::DBitV> *node)
    {
        sym_ = sym;
        node_ = node;
        patterns_.clear();
    }

    bool addPattern(const range_type &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
    }

    double score() const
    {
        return detail::getPatternScore(patterns_);
    }

public:
    uint64_t sym_;
    const WaveletTreeNode<dense::DBitV> *node_;
    range_list_type patterns_;
} __attribute__((aligned(CACHELINE_SIZE)));

template <class WaveletTreeType>
class FilterItem
{
public:
    FilterItem(
            const WaveletTreeType *tree, const WaveletTreeNode<dense::DBitV> *node,
            const range_list_type &segments,
            boost::auto_alloc& alloc)
        : tree_(tree) , node_(node)
        , segments_(alloc)
    {
        segments_.assign(segments.begin(), segments.end());
    }

    FilterItem(
            const WaveletTreeType *tree, const WaveletTreeNode<dense::DBitV> *node,
            size_t filter_count,
            boost::auto_alloc& alloc)
        : tree_(tree) , node_(node), segments_(alloc)
    {
        segments_.reserve(filter_count);
    }

    ~FilterItem() {}

    void reset(const WaveletTreeType *tree, const WaveletTreeNode<dense::DBitV> *node)
    {
        tree_ = tree;
        node_ = node;
        segments_.clear();
    }

    bool addSegment(const range_type &segment)
    {
        if (segment.get<0>() < segment.get<1>())
        {
            segments_.push_back(segment);
            return true;
        }
        return false;
    }

public:
    const WaveletTreeType *tree_;
    const WaveletTreeNode<dense::DBitV> *node_;
    range_list_type segments_;
} __attribute__((aligned(CACHELINE_SIZE)));

template <class WaveletTreeType>
class FilteredPatternList
{
public:
    typedef std::vector<FilterItem<WaveletTreeType> *, boost::stl_allocator<FilterItem<WaveletTreeType> *> > filter_list_type;

    FilteredPatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            const filter_list_type &filters,
            const range_list_type &patterns,
            boost::auto_alloc& alloc)
        : sym_(sym)
        , node_(node)
        , filters_(filters)
        , patterns_(patterns)
        , recyc_filters_(alloc)
        , alloc_(alloc)
    {
        if (!filters_.empty())
        {
            recyc_filters_.reserve(filters_.size());
        }
    }

    FilteredPatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            size_t filter_count, size_t pattern_count,
            boost::auto_alloc& alloc)
        : sym_(sym)
        , node_(node)
        , filters_(alloc)
        , patterns_(alloc)
        , recyc_filters_(alloc)
        , alloc_(alloc)
    {
        filters_.reserve(filter_count);
        patterns_.reserve(pattern_count);

        recyc_filters_.reserve(filter_count);
    }

    ~FilteredPatternList()
    {
        /*
        for (size_t i = 0; i < filters_.size(); ++i)
        {
            delete filters_[i];
        }

        for (size_t i = 0; i < recyc_filters_.size(); ++i)
        {
            delete recyc_filters_[i];
        }*/
    }

    void reset(uint64_t sym, const WaveletTreeNode<dense::DBitV> *node)
    {
        sym_ = sym;
        node_ = node;
        recyc_filters_.insert(recyc_filters_.end(), filters_.begin(), filters_.end());
        filters_.clear();
        patterns_.clear();
    }

    bool addFilter(FilterItem<WaveletTreeType> *filter)
    {
        assert(filter != NULL);

        if (filter->segments_.empty())
        {
            recyc_filters_.push_back(filter);
            return false;
        }
        else
        {
            filters_.push_back(filter);
            return true;
        }
    }

    bool addPattern(const range_type &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
    }

    FilterItem<WaveletTreeType> *getFilter(
            const WaveletTreeType *tree, const WaveletTreeNode<dense::DBitV> *node,
            size_t filter_count)
    {
        if (recyc_filters_.empty())
        {
            return BOOST_NEW(alloc_,FilterItem<WaveletTreeType>)(tree, node, filter_count, alloc_);
        }
        else
        {
            FilterItem<WaveletTreeType> *filter = recyc_filters_.back();
            filter->reset(tree, node);
            recyc_filters_.pop_back();
            return filter;
        }
    }

    double score() const
    {
        return detail::getPatternScore(patterns_);
    }


public:
    uint64_t sym_;
    const WaveletTreeNode<dense::DBitV> *node_;
    filter_list_type filters_;
    range_list_type patterns_;
    filter_list_type recyc_filters_;
    boost::auto_alloc& alloc_;
} __attribute__((aligned(CACHELINE_SIZE)));

class SynonymPatternList
{
public:
    SynonymPatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            const range_list_type &patterns,
            const head_list_type &synonyms)
        : sym_(sym)
        , node_(node)
        , patterns_(patterns)
        , synonyms_(synonyms)
    {
        for (size_t i = 0; i < synonyms_.size() - 1; ++i)
        {
            std::sort(patterns_.begin() + synonyms_[i], patterns_.begin() + synonyms_[i + 1], detail::range_compare);
        }
    }

    SynonymPatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            size_t pattern_count,
            size_t synonym_count,
            boost::auto_alloc& alloc)
        : sym_(sym)
        , node_(node)
        , patterns_(alloc)
        , synonyms_(alloc)
    {
        patterns_.reserve(pattern_count);
        synonyms_.reserve(synonym_count);
        synonyms_.push_back(0);
    }

    ~SynonymPatternList() {}

    void reset(uint64_t sym, const WaveletTreeNode<dense::DBitV> *node)
    {
        sym_ = sym;
        node_ = node;
        patterns_.clear();
        synonyms_.resize(1);
    }

    bool addPattern(const range_type &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
    }

    bool addSynonym()
    {
        if (patterns_.size() > synonyms_.back())
        {
            synonyms_.push_back(patterns_.size());
            return true;
        }
        return false;
    }

    double score() const
    {
        return detail::getSynonymPatternScore(patterns_, synonyms_);
    }

public:
    uint64_t sym_;
    const WaveletTreeNode<dense::DBitV> *node_;
    range_list_type patterns_;
    head_list_type synonyms_;
} __attribute__((aligned(CACHELINE_SIZE)));

template <class WaveletTreeType>
class FilteredSynonymPatternList
{
public:
    typedef std::vector<FilterItem<WaveletTreeType> *, boost::stl_allocator<FilterItem<WaveletTreeType> *> > filter_list_type;

    FilteredSynonymPatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            const filter_list_type &filters,
            const range_list_type &patterns,
            const head_list_type &synonyms,
            boost::auto_alloc& alloc)
        : sym_(sym)
        , node_(node)
        , filters_(filters)
        , patterns_(patterns)
        , synonyms_(synonyms)
        , recyc_filters_(alloc)
        , alloc_(alloc)
    {
        if (!filters_.empty())
        {
            for (size_t i = 0; i < synonyms_.size() - 1; ++i)
            {
                std::sort(patterns_.begin() + synonyms_[i], patterns_.begin() + synonyms_[i + 1], detail::range_compare);
            }
            recyc_filters_.reserve(filters_.size());
        }
    }

    FilteredSynonymPatternList(
            uint64_t sym,
            const WaveletTreeNode<dense::DBitV> *node,
            size_t filter_count,
            size_t pattern_count,
            size_t synonym_count,
            boost::auto_alloc& alloc)
        : sym_(sym)
        , node_(node)
        , filters_(alloc)
        , patterns_(alloc)
        , synonyms_(alloc)
        , recyc_filters_(alloc)
        , alloc_(alloc)
    {
        filters_.reserve(filter_count);
        patterns_.reserve(pattern_count);
        synonyms_.reserve(synonym_count);
        synonyms_.push_back(0);

        recyc_filters_.reserve(filter_count);
    }

    ~FilteredSynonymPatternList()
    {
        /*
        for (size_t i = 0; i < filters_.size(); ++i)
        {
            delete filters_[i];
        }

        for (size_t i = 0; i < recyc_filters_.size(); ++i)
        {
            delete recyc_filters_[i];
        }*/
    }

    void reset(uint64_t sym, const WaveletTreeNode<dense::DBitV> *node)
    {
        sym_ = sym;
        node_ = node;
        recyc_filters_.insert(recyc_filters_.end(), filters_.begin(), filters_.end());
        filters_.clear();
        patterns_.clear();
        synonyms_.resize(1);
    }

    bool addFilter(FilterItem<WaveletTreeType> *filter)
    {
        assert(filter != NULL);

        if (filter->segments_.empty())
        {
            recyc_filters_.push_back(filter);
            return false;
        }
        else
        {
            filters_.push_back(filter);
            return true;
        }
    }

    bool addPattern(const range_type &pattern)
    {
        if (pattern.get<0>() < pattern.get<1>())
        {
            patterns_.push_back(pattern);
            return true;
        }
        return false;
    }

    bool addSynonym()
    {
        if (patterns_.size() > synonyms_.back())
        {
            synonyms_.push_back(patterns_.size());
            return true;
        }
        return false;
    }

    FilterItem<WaveletTreeType> *getFilter(
            const WaveletTreeType *tree, const WaveletTreeNode<dense::DBitV> *node,
            size_t filter_count)
    {
        if (recyc_filters_.empty())
        {
            return BOOST_NEW(alloc_,FilterItem<WaveletTreeType>)(tree, node, filter_count, alloc_);
        }
        else
        {
            FilterItem<WaveletTreeType> *filter = recyc_filters_.back();
            filter->reset(tree, node);
            recyc_filters_.pop_back();
            return filter;
        }
    }

    double score() const
    {
        return detail::getSynonymPatternScore(patterns_, synonyms_);
    }

public:
    uint64_t sym_;
    const WaveletTreeNode<dense::DBitV> *node_;
    filter_list_type filters_;
    range_list_type patterns_;
    head_list_type synonyms_;
    filter_list_type recyc_filters_;
    boost::auto_alloc& alloc_;
} __attribute__((aligned(CACHELINE_SIZE)));

}
}

NS_IZENELIB_AM_END

namespace std
{

template <class T>
struct less<boost::tuple<float, uint32_t, T*> >
{
    bool operator()(boost::tuple<float, uint32_t, T*> const &p1, boost::tuple<float, uint32_t, T*> const &p2)
    {
        return boost::get<0>(p1) < boost::get<0>(p2) || (boost::get<0>(p1) == boost::get<0>(p2) && boost::get<1>(p1) < boost::get<1>(p2));
    }
};

}

#endif
