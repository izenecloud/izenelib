#include <am/succinct/fm-index/range_list.hpp>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

double getScore(const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
{
    double score = 0.0;

    for (std::vector<boost::tuple<size_t, size_t, double> >::const_iterator it = ranges.begin();
            it != ranges.end(); ++it)
    {
        if (it->get<1>() > it->get<0>())
            score += it->get<2>();
    }

    return score;
}

RangeList::RangeList(size_t level, uint64_t sym, const WaveletTreeNode *node, const std::vector<boost::tuple<size_t, size_t, double> > &ranges)
    : level_(level)
    , sym_(sym)
    , score_(getScore(ranges))
    , node_(node)
    , ranges_(ranges)
{
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
    ranges_.push_back(range);
}

void RangeList::calcScore()
{
    score_ = getScore(ranges_);
}

bool RangeList::operator<(const RangeList &rhs) const
{
    if (score_ < rhs.score_)
    {
        return true;
    }
    else if (score_ == rhs.score_)
    {
        return level_ < rhs.level_;
    }
    else
    {
        return false;
    }
}

}
}

NS_IZENELIB_AM_END
