#ifndef _FM_INDEX_WAVELET_TREE_BINARY_HPP
#define _FM_INDEX_WAVELET_TREE_BINARY_HPP

#include "wavelet_tree.hpp"
#include <am/succinct/sdarray/SDArray.hpp>
#include <am/succinct/utils.hpp>
#include <am/interval_heap.hpp>

#include <deque>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class WaveletTreeBinary : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;
    typedef WaveletTreeBinary<CharT> self_type;
    typedef std::vector<FilterList<self_type> *, boost::stl_allocator<FilterList<self_type> *> > aux_filter_list_type;

    WaveletTreeBinary(uint64_t alphabet_num, bool support_select, bool dense);
    ~WaveletTreeBinary();

    void build(const char_type *char_seq, size_t len);

    char_type access(size_t pos) const;
    char_type access(size_t pos, size_t &rank) const;

    size_t rank(char_type c, size_t pos) const;
    size_t select(char_type c, size_t rank) const;

    void intersect(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &results) const;

    void topKUnion(
            const range_list_type &patterns,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    void topKUnionWithFilters(
            const range_list_type &filters,
            const range_list_type &patterns,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    void topKUnionWithAuxFilters(
            const aux_filter_list_type &aux_filters,
            const std::vector<std::pair<size_t, size_t> > &filters,
            const range_list_type &patterns,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    size_t getOcc(char_type c) const;
    WaveletTreeNode *getRoot() const;

    size_t length() const;
    size_t allocSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    void doIntersect_(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            size_t level,
            size_t start,
            char_type symbol,
            std::vector<char_type> &results) const;

private:
    sdarray::SDArray occ_;
    std::vector<WaveletTreeNode *> nodes_;
};

template <class CharT>
WaveletTreeBinary<CharT>::WaveletTreeBinary(uint64_t alphabet_num, bool support_select, bool dense)
    : WaveletTree<CharT>(alphabet_num, support_select, dense)
{
}

template <class CharT>
WaveletTreeBinary<CharT>::~WaveletTreeBinary()
{
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    this->alphabet_bit_num_ = SuccinctUtils::log2(this->alphabet_num_ - 1);

    std::vector<std::vector<size_t> > beg_poses(this->alphabet_bit_num_ + 1);
    beg_poses[0].resize(1);
    for (size_t i = 1; i < beg_poses.size(); ++i)
    {
        beg_poses[i].resize(((this->alphabet_num_ - 1) >> (this->alphabet_bit_num_ - i)) + 2);
    }
    for (size_t i = 0; i < len; ++i)
    {
        const char_type &c = char_seq[i];
        for (size_t j = 1; j < beg_poses.size(); ++j)
        {
            ++beg_poses[j][(c >> (this->alphabet_bit_num_ - j)) + 1];
        }
    }
    for (size_t i = 1; i < beg_poses.size() - 1; ++i)
    {
        std::vector<size_t>& beg_poses_level = beg_poses[i];
        for (size_t j = 2; j < beg_poses_level.size(); ++j)
        {
            beg_poses_level[j] += beg_poses_level[j - 1];
        }
    }

    for (size_t i = 1; i <= this->alphabet_num_; ++i)
    {
        occ_.add(beg_poses.back()[i]);
    }
    beg_poses.pop_back();
    occ_.build();

    nodes_.resize(this->alphabet_bit_num_);
    for (size_t i = 0; i < this->alphabet_bit_num_; ++i)
    {
        nodes_[i] = new WaveletTreeNode(this->support_select_, this->dense_);
        nodes_[i]->resize(len);
    }
    for (size_t i = 0; i < len; ++i)
    {
        const char_type &c = char_seq[i];
        nodes_[0]->changeBit(c >> (this->alphabet_bit_num_ - 1) & (char_type)1, beg_poses[0][0]++);
        for (size_t j = 1; j < beg_poses.size(); ++j)
        {
            nodes_[j]->changeBit(c >> (this->alphabet_bit_num_ - j - 1) & (char_type)1,
                                 beg_poses[j][c >> (this->alphabet_bit_num_ - j)]++);
        }
    }

    std::vector<std::vector<size_t> >().swap(beg_poses);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->build();
    }
    for (size_t i = 1; i < nodes_.size(); ++i)
    {
        nodes_[i]->parent_ = nodes_[i - 1];
        nodes_[i - 1]->left_ = nodes_[i];
        nodes_[i - 1]->right_ = nodes_[i];
    }
}

template <class CharT>
CharT WaveletTreeBinary<CharT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    size_t start = 0;
    size_t optR, before;

    char_type c = 0;
    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const WaveletTreeNode *node = nodes_[i];

        before = node->rank1(start);

        if (node->access(pos, optR))
        {
            c |= bit_mask;
            start = occ_.prefixSum(c);
            pos = start - before + optR;
        }
        else
        {
            pos = before + optR;
        }

        bit_mask >>= 1;
    }

    return c;
}

template <class CharT>
CharT WaveletTreeBinary<CharT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    size_t start = 0;
    size_t optR, before;

    char_type c = 0;
    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const WaveletTreeNode *node = nodes_[i];

        before = node->rank1(start);

        if (node->access(pos, optR))
        {
            c |= bit_mask;
            start = occ_.prefixSum(c);
            pos = start - before + optR;
        }
        else
        {
            pos = before + optR;
        }

        bit_mask >>= 1;
    }

    rank = pos - start;

    return c;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::rank(char_type c, size_t pos) const
{
    size_t start = 0;
    size_t rank = 0;
    size_t before;

    pos = std::min(pos, length());

    char_type masked = 0;
    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        const WaveletTreeNode *node = nodes_[i];

        before = node->rank1(start);

        if (c & bit_mask)
        {
            masked |= bit_mask;
            start = occ_.prefixSum(masked);
            rank = node->rank1(pos) - before;
            pos = start + rank;
        }
        else
        {
            pos = pos - node->rank1(pos) + before;
            rank = pos - start;
        }

        if (rank == 0) return 0;

        bit_mask >>= 1;
    }

    return rank;
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::select(char_type c, size_t rank) const
{
    size_t start, ones_start;

    char_type mask = ((char_type)1 << this->alphabet_bit_num_) - 2;
    char_type bit_mask = 1;

    for (size_t i = nodes_.size() - 1; i < nodes_.size(); --i)
    {
        const WaveletTreeNode *node = nodes_[i];

        start = occ_.prefixSum(c & mask);
        ones_start = node->rank1(start);

        if (c & bit_mask)
        {
            rank = node->select1(ones_start + rank) - start;
        }
        else
        {
            rank = node->select0(start - ones_start + rank) - start;
        }

        if (rank == (size_t)-1) return -1;

        mask <<= 1;
        bit_mask <<= 1;
    }

    return rank;
}

template <class CharT>
void WaveletTreeBinary<CharT>::intersect(
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        std::vector<char_type> &results) const
{
    if (thres > patterns.size()) return;
    if (thres > 0) thres = patterns.size() - thres;

    doIntersect_(patterns, thres, max_count, 0, 0, 0, results);
}

template <class CharT>
void WaveletTreeBinary<CharT>::doIntersect_(
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        size_t level,
        size_t start,
        char_type symbol,
        std::vector<char_type> &results) const
{
    if (results.size() >= max_count) return;

    if (level == this->alphabet_bit_num_)
    {
        results.push_back(symbol);
        return;
    }

    std::vector<std::pair<size_t, size_t> > zero_ranges, one_ranges;
    zero_ranges.reserve(patterns.size());
    one_ranges.reserve(patterns.size());

    size_t zero_thres = thres, one_thres = thres;
    bool has_zeros = true, has_ones = true;

    const WaveletTreeNode *node = nodes_[level];

    size_t before = node->rank1(start);
    size_t rank_start, rank_end;

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = patterns.begin();
            it != patterns.end(); ++it)
    {
        rank_start = node->rank1(start + it->first);
        rank_end = node->rank1(start + it->second);

        if (has_zeros)
        {
            if (it->first - rank_start >= it->second - rank_end)
            {
                if (zero_thres == 0)
                {
                    if (!has_ones) return;
                    has_zeros = false;
                }
                else
                {
                    --zero_thres;
                }
            }
            else
            {
                zero_ranges.push_back(std::make_pair(it->first - rank_start + before, it->second - rank_end + before));
            }
        }

        if (has_ones)
        {
            if (rank_start >= rank_end)
            {
                if (one_thres == 0)
                {
                    if (!has_zeros) return;
                    has_ones = false;
                }
                else
                {
                    --one_thres;
                }
            }
            else
            {
                one_ranges.push_back(std::make_pair(rank_start - before, rank_end - before));
            }
        }
    }

    if (has_zeros)
    {
        doIntersect_(zero_ranges, zero_thres, max_count, level + 1, start, symbol, results);
    }

    if (has_ones)
    {
        symbol |= (char_type)1 << (this->alphabet_bit_num_ - level - 1);
        start = occ_.prefixSum(symbol);

        doIntersect_(one_ranges, one_thres, max_count, level + 1, start, symbol, results);
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::topKUnion(
        const range_list_type &patterns,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<std::pair<PatternList *, size_t> > ranges_heap(max_queue_size + 1);
    //ranges_heap.insert(std::make_pair(new PatternList(0, (char_type)0, nodes_[0], patterns, alloc), 0));
    ranges_heap.insert(std::make_pair(BOOST_NEW(alloc, PatternList)(0, (char_type)0, nodes_[0], patterns), 0));
    if (ranges_heap.get_max().first->score_ == 0.0)
    {
        //delete ranges_heap.get_max().first;
        return;
    }

    results.reserve(topK);

    std::vector<PatternList *, boost::stl_allocator<PatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<std::pair<PatternList *, size_t>, boost::stl_allocator<std::pair<PatternList *, size_t> > > top_queue(alloc);

    PatternList *top_ranges;
    PatternList *zero_ranges, *one_ranges;
    size_t start, before, rank_start, rank_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back().first;
            start = top_queue.back().second;
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max().first;
            start = ranges_heap.get_max().second;
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        node = top_ranges->node_;
        before = node->rank1(start);

        if (recyc_queue.empty())
        {
            //zero_ranges = new PatternList(top_ranges->level_ + 1, top_ranges->sym_, node->left_, top_ranges->patterns_.capacity(), alloc);
            zero_ranges = BOOST_NEW(alloc, PatternList)(top_ranges->level_ + 1, top_ranges->sym_, node->left_, top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->level_ + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            //one_ranges = new PatternList(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_, top_ranges->patterns_.capacity(), alloc);
            one_ranges = BOOST_NEW(alloc, PatternList)(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_, top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_);
            recyc_queue.pop_back();
        }

        for (range_list_type::const_iterator it = top_ranges->patterns_.begin();
                it != top_ranges->patterns_.end(); ++it)
        {
            rank_start = node->rank1(start + it->get<0>()) - before;
            rank_end = node->rank1(start + it->get<1>()) - before;

            zero_ranges->addPattern(boost::make_tuple(it->get<0>() - rank_start, it->get<1>() - rank_end, it->get<2>()));
            one_ranges->addPattern(boost::make_tuple(rank_start, rank_end, it->get<2>()));
        }

        zero_ranges->calcScore();
        if (zero_ranges->score_ == 0.0)
        {
            recyc_queue.push_back(zero_ranges);
        }
        else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max().first->score_)))
        {
            if (zero_ranges->node_)
            {
                top_queue.push_back(std::make_pair(zero_ranges, start));
            }
            else
            {
                results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                recyc_queue.push_back(zero_ranges);
            }
        }
        else
        {
            ranges_heap.insert(std::make_pair(zero_ranges, start));
        }

        one_ranges->calcScore();
        if (one_ranges->score_ == 0.0)
        {
            recyc_queue.push_back(one_ranges);
        }
        else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max().first->score_)))
        {
            if (one_ranges->node_)
            {
                top_queue.push_back(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));

                if (top_queue.size() > max_queue_size)
                {
                    recyc_queue.push_back(top_queue.front().first);
                    top_queue.pop_front();
                }
                else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min().first);
                    ranges_heap.pop_min();
                }
            }
            else
            {
                results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                recyc_queue.push_back(one_ranges);
            }
        }
        else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min().first->score_))
        {
            recyc_queue.push_back(one_ranges);
        }
        else
        {
            ranges_heap.insert(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));

            if (top_queue.size() + ranges_heap.size() > max_queue_size)
            {
                recyc_queue.push_back(ranges_heap.get_min().first);
                ranges_heap.pop_min();
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    interval_heap<std::pair<PatternList *, size_t> >::container_type ranges_list = ranges_heap.get_container();
    for (size_t i = 0; i < ranges_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first.first;
        delete ranges_list[i].second.first;
    }
    if (ranges_heap.size() % 2)
    {
        delete ranges_list[ranges_heap.size() / 2].first.first;
    }

    for (size_t i = 0; i < recyc_queue.size(); ++i)
    {
        delete recyc_queue[i];
    }

    for (size_t i = 0; i < top_queue.size(); ++i)
    {
        delete top_queue[i].first;
    }*/
}

template <class CharT>
void WaveletTreeBinary<CharT>::topKUnionWithFilters(
        const range_list_type &filters,
        const range_list_type &patterns,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<std::pair<FilteredPatternList *, size_t> > ranges_heap(max_queue_size + 1);
    //ranges_heap.insert(std::make_pair(new FilteredPatternList(0, (char_type)0, nodes_[0], filters, patterns), 0));
    ranges_heap.insert(std::make_pair(BOOST_NEW(alloc, FilteredPatternList)(0, (char_type)0, nodes_[0], filters, patterns), 0));

    if (ranges_heap.get_max().first->score_ == 0.0)
    {
        //delete ranges_heap.get_max().first;
        return;
    }

    results.reserve(topK);

    std::vector<FilteredPatternList *, boost::stl_allocator<FilteredPatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<std::pair<FilteredPatternList *, size_t>, boost::stl_allocator<std::pair<FilteredPatternList *, size_t> > > top_queue(alloc);

    FilteredPatternList *top_ranges;
    FilteredPatternList *zero_ranges, *one_ranges;
    size_t start, before, rank_start, rank_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back().first;
            start = top_queue.back().second;
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max().first;
            start = ranges_heap.get_max().second;
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        node = top_ranges->node_;
        before = node->rank1(start);

        if (recyc_queue.empty())
        {
            //zero_ranges = new FilteredPatternList(top_ranges->level_ + 1, top_ranges->sym_, node->left_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
            zero_ranges = BOOST_NEW(alloc, FilteredPatternList)(top_ranges->level_ + 1, top_ranges->sym_, node->left_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->level_ + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            //one_ranges = new FilteredPatternList(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
            one_ranges = BOOST_NEW(alloc, FilteredPatternList)(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_);
            recyc_queue.pop_back();
        }

        for (range_list_type::const_iterator it = top_ranges->filters_.begin();
                it != top_ranges->filters_.end(); ++it)
        {
            rank_start = node->rank1(start + it->get<0>()) - before;
            rank_end = node->rank1(start + it->get<1>()) - before;

            zero_ranges->addFilter(boost::make_tuple(it->get<0>() - rank_start, it->get<1>() - rank_end, it->get<2>()));
            one_ranges->addFilter(boost::make_tuple(rank_start, rank_end, it->get<2>()));
        }

        if (zero_ranges->filters_.empty())
        {
            recyc_queue.push_back(zero_ranges);
            zero_ranges = NULL;
        }
        if (one_ranges->filters_.empty())
        {
            recyc_queue.push_back(one_ranges);
            one_ranges = NULL;
        }
        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        for (range_list_type::const_iterator it = top_ranges->patterns_.begin();
                it != top_ranges->patterns_.end(); ++it)
        {
            rank_start = node->rank1(start + it->get<0>()) - before;
            rank_end = node->rank1(start + it->get<1>()) - before;

            if (zero_ranges)
            {
                zero_ranges->addPattern(boost::make_tuple(it->get<0>() - rank_start, it->get<1>() - rank_end, it->get<2>()));
            }
            if (one_ranges)
            {
                one_ranges->addPattern(boost::make_tuple(rank_start, rank_end, it->get<2>()));
            }
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max().first->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(std::make_pair(zero_ranges, start));
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(std::make_pair(zero_ranges, start));
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max().first->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front().first);
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min().first);
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min().first->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min().first);
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    interval_heap<std::pair<FilteredPatternList *, size_t> >::container_type ranges_list = ranges_heap.get_container();
    for (size_t i = 0; i < ranges_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first.first;
        delete ranges_list[i].second.first;
    }
    if (ranges_heap.size() % 2)
    {
        delete ranges_list[ranges_heap.size() / 2].first.first;
    }

    for (size_t i = 0; i < recyc_queue.size(); ++i)
    {
        delete recyc_queue[i];
    }

    for (size_t i = 0; i < top_queue.size(); ++i)
    {
        delete top_queue[i].first;
    }*/
}

template <class CharT>
void WaveletTreeBinary<CharT>::topKUnionWithAuxFilters(
        const aux_filter_list_type &aux_filters,
        const std::vector<std::pair<size_t, size_t> > &filters,
        const range_list_type &patterns,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<std::pair<AuxFilteredPatternList<self_type> *, size_t> > ranges_heap(max_queue_size + 1);
    //ranges_heap.insert(std::make_pair(new AuxFilteredPatternList<self_type>(0, (char_type)0, nodes_[0], aux_filters, filters, patterns, alloc), 0));
    ranges_heap.insert(std::make_pair(BOOST_NEW(alloc, AuxFilteredPatternList<self_type>)(0, (char_type)0, nodes_[0], aux_filters, filters, patterns, alloc), 0));
    if (ranges_heap.get_max().first->score_ == 0.0)
    {
        //delete ranges_heap.get_max().first;
        return;
    }

    results.reserve(topK);

    size_t max_filter_size = 0;
    for (size_t i = 0; i < aux_filters.size(); ++i)
    {
        max_filter_size = std::max(max_filter_size, aux_filters[i]->filters_.size());
    }

    std::vector<AuxFilteredPatternList<self_type> *, boost::stl_allocator<AuxFilteredPatternList<self_type> *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<std::pair<AuxFilteredPatternList<self_type> *, size_t>, boost::stl_allocator<std::pair<AuxFilteredPatternList<self_type> *, size_t> > > top_queue(alloc);

    AuxFilteredPatternList<self_type> *top_ranges;
    AuxFilteredPatternList<self_type> *zero_ranges, *one_ranges;
    FilterList<self_type> *zero_filter, *one_filter;
    size_t start, filter_start, before, rank_start, rank_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back().first;
            start = top_queue.back().second;
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max().first;
            start = ranges_heap.get_max().second;
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            //zero_ranges = new AuxFilteredPatternList<self_type>(top_ranges->level_ + 1, top_ranges->sym_, node->left_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
            zero_ranges = BOOST_NEW(alloc, AuxFilteredPatternList<self_type>)(top_ranges->level_ + 1, top_ranges->sym_, node->left_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->level_ + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            //one_ranges = new AuxFilteredPatternList<self_type>(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
            one_ranges = BOOST_NEW(alloc, AuxFilteredPatternList<self_type>)(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(zero_ranges->level_, top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - zero_ranges->level_), node->right_);
            recyc_queue.pop_back();
        }

        for (typename aux_filter_list_type::const_iterator it = top_ranges->aux_filters_.begin();
                it != top_ranges->aux_filters_.end(); ++it)
        {
            node = (*it)->node_;
            filter_start = (*it)->tree_->occ_.prefixSum(top_ranges->sym_);
            before = node->rank1(filter_start);

            if (zero_ranges)
            {
                zero_filter = zero_ranges->getAuxFilter((*it)->tree_, node->left_, max_filter_size);
            }
            if (one_ranges)
            {
                one_filter = one_ranges->getAuxFilter((*it)->tree_, node->right_, max_filter_size);
            }

            for (range_list_type::const_iterator fit = (*it)->filters_.begin();
                    fit != (*it)->filters_.end(); ++fit)
            {
                rank_start = node->rank1(filter_start + fit->get<0>()) - before;
                rank_end = node->rank1(filter_start + fit->get<1>()) - before;

                if (zero_ranges)
                {
                    zero_filter->addFilter(boost::make_tuple(fit->get<0>() - rank_start, fit->get<1>() - rank_end, fit->get<2>()));
                }
                if (one_ranges)
                {
                    one_filter->addFilter(boost::make_tuple(rank_start, rank_end, fit->get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addAuxFilter(zero_filter))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addAuxFilter(one_filter))
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        node = top_ranges->node_;
        before = node->rank1(start);

        for (range_list_type::const_iterator it = top_ranges->patterns_.begin();
                it != top_ranges->patterns_.end(); ++it)
        {
            rank_start = node->rank1(start + it->get<0>()) - before;
            rank_end = node->rank1(start + it->get<1>()) - before;

            if (zero_ranges)
            {
                zero_ranges->addPattern(boost::make_tuple(it->get<0>() - rank_start, it->get<1>() - rank_end, it->get<2>()));
            }
            if (one_ranges)
            {
                one_ranges->addPattern(boost::make_tuple(rank_start, rank_end, it->get<2>()));
            }
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max().first->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(std::make_pair(zero_ranges, start));
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(std::make_pair(zero_ranges, start));
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max().first->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front().first);
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min().first);
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min().first->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(std::make_pair(one_ranges, occ_.prefixSum(one_ranges->sym_)));

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min().first);
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    typename interval_heap<std::pair<AuxFilteredPatternList<self_type> *, size_t> >::container_type ranges_list = ranges_heap.get_container();
    for (size_t i = 0; i < ranges_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first.first;
        delete ranges_list[i].second.first;
    }
    if (ranges_heap.size() % 2)
    {
        delete ranges_list[ranges_heap.size() / 2].first.first;
    }

    for (size_t i = 0; i < recyc_queue.size(); ++i)
    {
        delete recyc_queue[i];
    }

    for (size_t i = 0; i < top_queue.size(); ++i)
    {
        delete top_queue[i].first;
    }*/
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::getOcc(char_type c) const
{
    if (c <= occ_.size()) return occ_.prefixSum(c);
    return occ_.getSum();
}

template <class CharT>
WaveletTreeNode *WaveletTreeBinary<CharT>::getRoot() const
{
    if (nodes_.empty()) return NULL;
    else return nodes_[0];
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::length() const
{
    return nodes_.empty() ? 0 : nodes_[0]->length();
}

template <class CharT>
size_t WaveletTreeBinary<CharT>::allocSize() const
{
    size_t sum = sizeof(WaveletTreeBinary<char_type>) + occ_.allocSize();
    for (size_t i = 0; i < nodes_.size(); ++i)
        sum += nodes_[i]->allocSize();

    return sum;
}

template <class CharT>
void WaveletTreeBinary<CharT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);
    occ_.save(ostr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->save(ostr);
    }
}

template <class CharT>
void WaveletTreeBinary<CharT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);
    occ_.load(istr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }

    this->alphabet_bit_num_ = SuccinctUtils::log2(this->alphabet_num_ - 1);

    nodes_.resize(this->alphabet_bit_num_);
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i] = new WaveletTreeNode(this->support_select_, this->dense_);
        nodes_[i]->load(istr);
    }
    for (size_t i = 1; i < nodes_.size(); ++i)
    {
        nodes_[i]->parent_ = nodes_[i - 1];
        nodes_[i - 1]->left_ = nodes_[i];
        nodes_[i - 1]->right_ = nodes_[i];
    }
}

}
}

NS_IZENELIB_AM_END

#endif
