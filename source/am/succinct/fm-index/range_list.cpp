#include <am/succinct/fm-index/range_list.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

namespace detail
{

double getScore(const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
{
    double score = 0.0;

    for (std::vector<boost::tuple<size_t, size_t, double> >::const_iterator it = ranges.begin();
            it != ranges.end(); ++it)
    {
        score += it->get<2>();
    }

    return score;
}

static struct InvalidRange
{
    bool operator()(const std::pair<size_t, size_t> &value) const
    {
        return value.first >= value.second;
    }

    bool operator()(const boost::tuple<size_t, size_t, double> &value) const
    {
        return value.get<0>() >= value.get<1>();
    }
} InvalidRange;

}

RangeList::RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node, const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
    : level_(level)
    , sym_(sym)
    , node_(node)
    , ranges_(ranges)
{
    ranges_.erase(std::remove_if(ranges_.begin(), ranges_.end(), detail::InvalidRange), ranges_.end());
    score_ = detail::getScore(ranges_);
}

RangeList::RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node, size_t capacity)
    : level_(level)
    , sym_(sym)
    , score_()
    , node_(node)
{
    ranges_.reserve(capacity);
}

RangeList::~RangeList()
{
}

void RangeList::addRange(const boost::tuple<size_t, size_t, double> &range)
{
    if (range.get<0>() < range.get<1>())
        ranges_.push_back(range);
}

void RangeList::calcScore()
{
    score_ = detail::getScore(ranges_);
}

bool RangeList::operator<(const RangeList &rhs) const
{
    return score_ < rhs.score_ || (score_ == rhs.score_ && level_ < rhs.level_);
}

FilteredRangeList::FilteredRangeList(
        size_t level, uint64_t sym,
        const WaveletTreeNode *node,
        const std::vector<std::pair<size_t, size_t> > &filters,
        const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
    : level_(level)
    , sym_(sym)
    , score_()
    , node_(node)
    , filters_(filters)
    , ranges_(ranges)
{
    filters_.erase(std::remove_if(filters_.begin(), filters_.end(), detail::InvalidRange), filters_.end());
    if (!filters_.empty())
    {
        ranges_.erase(std::remove_if(ranges_.begin(), ranges_.end(), detail::InvalidRange), ranges_.end());
        score_ = detail::getScore(ranges_);
    }
}

FilteredRangeList::FilteredRangeList(
        size_t level, uint64_t sym,
        const WaveletTreeNode *node,
        size_t filter_count, size_t range_count)
    : level_(level)
    , sym_(sym)
    , score_()
    , node_(node)
{
    filters_.reserve(filter_count);
    ranges_.reserve(range_count);
}

FilteredRangeList::~FilteredRangeList()
{
}

void FilteredRangeList::addFilter(const std::pair<size_t, size_t> &filter)
{
    if (filter.first < filter.second)
        filters_.push_back(filter);
}

void FilteredRangeList::addRange(const boost::tuple<size_t, size_t, double> &range)
{
    if (range.get<0>() < range.get<1>())
        ranges_.push_back(range);
}

void FilteredRangeList::calcScore()
{
    score_ = detail::getScore(ranges_);
}

bool FilteredRangeList::operator<(const FilteredRangeList &rhs) const
{
    return score_ < rhs.score_ || (score_ == rhs.score_ && level_ < rhs.level_);
}

}
}

NS_IZENELIB_AM_END